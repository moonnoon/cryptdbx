#include <main/rewrite_util.hh>
#include <main/enum_text.hh>
#include <main/rewrite_main.hh>
#include <main/init_onions.hh>
#include <parser/lex_util.hh>
#include <parser/stringify.hh>
#include <List_helpers.hh>

extern CItemTypesDir itemTypes;

void
optimize(Item **i, Analysis &a) {
   //TODO
/*Item *i0 = itemTypes.do_optimize(*i, a);
    if (i0 != *i) {
        // item i was optimized (replaced) by i0
        if (a.itemRewritePlans.find(*i) != a.itemRewritePlans.end()) {
            a.itemRewritePlans[i0] = a.itemRewritePlans[*i];
            a.itemRewritePlans.erase(*i);
        }
        *i = i0;
    } */
}


// this function should be called at the root of a tree of items
// that should be rewritten
// @context defaults to empty string.
Item *
rewrite(Item *i, const OLK & constr, Analysis &a, std::string context)
{
    if (context.size()) {
        context = " for " + context;
    }
    RewritePlan * const rp = getAssert(a.rewritePlans, i);
    assert(rp);
    if (!rp->es_out.contains(constr)) {
        std::cerr << "query cannot be supported because " << i
                  << " needs to return " << constr << context << "\n"
                  << "BUT it can only return " << rp->es_out
                  << " BECAUSE " << rp->r << "\n";
        assert(false);
    }
    return itemTypes.do_rewrite(i, constr, rp, a);
}

TABLE_LIST *
rewrite_table_list(TABLE_LIST *t, Analysis &a)
{
    // Table name can only be empty when grouping a nested join.
    assert(t->table_name || t->nested_join);
    if (t->table_name) {
        std::string anon_name =
            a.getAnonTableName(std::string(t->table_name,
                               t->table_name_length));
        return rewrite_table_list(t, anon_name);
    } else {
        return copy(t);
    }
}

TABLE_LIST *
rewrite_table_list(TABLE_LIST *t, std::string anon_name)
{
    TABLE_LIST * const new_t = copy(t);
    new_t->table_name =
        make_thd_string(anon_name, &new_t->table_name_length);
    new_t->alias = make_thd_string(anon_name);
    new_t->next_local = NULL;

    return new_t;
}

// @if_exists: defaults to false.
SQL_I_List<TABLE_LIST>
rewrite_table_list(SQL_I_List<TABLE_LIST> tlist, Analysis &a,
                   bool if_exists)
{
    if (!tlist.elements) {
        return SQL_I_List<TABLE_LIST>();
    }

    TABLE_LIST * tl;
    if (if_exists && (false == a.tableMetaExists(tlist.first->table_name))) {
       tl = copy(tlist.first);
    } else {
       tl = rewrite_table_list(tlist.first, a);
    }

    SQL_I_List<TABLE_LIST> * new_tlist = oneElemList<TABLE_LIST>(tl);

    TABLE_LIST * prev = tl;
    for (TABLE_LIST *tbl = tlist.first->next_local; tbl;
         tbl = tbl->next_local) {
        TABLE_LIST * new_tbl;
        if (if_exists && (false == a.tableMetaExists(tbl->table_name))) {
            new_tbl = copy(tbl);
        } else {
            new_tbl = rewrite_table_list(tbl, a);
        }

        prev->next_local = new_tbl;
        prev = new_tbl;
    }
    prev->next_local = NULL;

    return *new_tlist;
}

List<TABLE_LIST>
rewrite_table_list(List<TABLE_LIST> tll, Analysis & a)
{
    List<TABLE_LIST> * const new_tll = new List<TABLE_LIST>();

    List_iterator<TABLE_LIST> join_it(tll);

    for (;;) {
        TABLE_LIST *t = join_it++;
        if (!t) {
            break;
        }

        TABLE_LIST * const new_t = rewrite_table_list(t, a);
        new_tll->push_back(new_t);

        if (t->nested_join) {
            new_t->nested_join->join_list = rewrite_table_list(t->nested_join->join_list, a);
            return *new_tll;
        }

        if (t->on_expr) {
            new_t->on_expr = rewrite(t->on_expr, PLAIN_OLK, a);
        }

	/* TODO: derived tables
        if (t->derived) {
            st_select_lex_unit *u = t->derived;
            rewrite_select_lex(u->first_select(), a);
        }
	*/
    }

    return *new_tll;
}

/*
 * Helper functions to look up via directory & invoke method.
 */
RewritePlan *
gather(Item *i, reason &tr, Analysis & a)
{
    return itemTypes.do_gather(i, tr, a);
}

//TODO: need to check somewhere that plain is returned
//TODO: Put in gather helpers file.
void
analyze(Item *i, Analysis & a)
{
    assert(i != NULL);
    LOG(cdb_v) << "calling gather for item " << *i;
    reason r;
    a.rewritePlans[i] = gather(i, r, a);
}

LEX *
begin_transaction_lex(const std::string &dbname) {
    static const std::string query = "START TRANSACTION;";
    query_parse *begin_parse = new query_parse(dbname, query);
    return begin_parse->lex();
}

LEX *
commit_transaction_lex(const std::string &dbname) {
    static const std::string query = "COMMIT;";
    query_parse *commit_parse = new query_parse(dbname, query);
    return commit_parse->lex();
}

//TODO(raluca) : figure out how to create Create_field from scratch
// and avoid this chaining and passing f as an argument
static Create_field *
get_create_field(const Analysis &a, Create_field * f, OnionMeta *om,
                 const std::string & name)
{
    Create_field * new_cf = f;
    
    auto enc_layers = a.getEncLayers(om);
    for (auto l : enc_layers) {
        Create_field * old_cf = new_cf;
        new_cf = l->newCreateField(old_cf, name);

        if (old_cf != f) {
            delete old_cf;
        }
    }

    return new_cf;
}

std::vector<Create_field *>
rewrite_create_field(FieldMeta *fm, Create_field *f, const Analysis &a)
{
    LOG(cdb_v) << "in rewrite create field for " << *f;

    std::vector<Create_field *> output_cfields;

    // FIXME: This sequence checking for encryption is broken.
    if (!fm->isEncrypted()) {
        // Unencrypted field
        output_cfields.push_back(f);
        return output_cfields;
    }

    // Encrypted field

    //check if field is not encrypted
    if (fm->children.empty()) {
        output_cfields.push_back(f);
        //cerr << "onions were empty" << endl;
        return output_cfields;
    }

    // create each onion column
    for (auto oit : fm->orderedOnionMetas()) {
        OnionMeta *om = oit.second;
        Create_field * new_cf =
        get_create_field(a, f, om, om->getAnonOnionName());
        /*
        EncLayer * last_layer = oit->second->layers.back();
        //create field with anonymous name
        Create_field * new_cf =
                last_layer->newCreateField(f, oit->second->getAnonOnionName().c_str());
        */
        output_cfields.push_back(new_cf);
    }

    // create salt column
    if (fm->has_salt) {
        //cerr << fm->salt_name << endl;
        THD *thd         = current_thd;
        Create_field *f0 = f->clone(thd->mem_root);
        f0->field_name   = thd->strdup(fm->getSaltName().c_str());
        f0->flags = f0->flags | UNSIGNED_FLAG;//salt is unsigned
        f0->sql_type     = MYSQL_TYPE_LONGLONG;
        f0->length       = 8;
        output_cfields.push_back(f0);
    }

    return output_cfields;
}

// TODO: Add Key for oDET onion as well.
std::vector<Key*>
rewrite_key(const std::string &table, Key *key, Analysis &a)
{
    std::vector<Key*> output_keys;
    Key * const new_key = key->clone(current_thd->mem_root);    
    auto col_it =
        List_iterator<Key_part_spec>(key->columns);
    // FIXME: Memleak.
    new_key->name =
        string_to_lex_str(a.getAnonIndexName(table, convert_lex_str(key->name)));
    new_key->columns = 
        reduceList<Key_part_spec>(col_it, List<Key_part_spec>(),
            [table, a] (List<Key_part_spec> out_field_list,
                        Key_part_spec *key_part) {
                const std::string field_name =
                    convert_lex_str(key_part->field_name);
                const FieldMeta * const fm =
                    a.getFieldMeta(table, field_name);
                const OnionMeta * const om = fm->getOnionMeta(oOPE);
                key_part->field_name =
                    string_to_lex_str(om->getAnonOnionName());
                out_field_list.push_back(key_part);
                return out_field_list; /* lambda */
            });
    output_keys.push_back(new_key);

    return output_keys;
}

std::string
bool_to_string(bool b)
{
    if (true == b) {
        return "TRUE";
    } else {
        return "FALSE";
    }
}

bool
string_to_bool(std::string s)
{
    if (s == std::string("TRUE") || s == std::string("1")) {
        return true;
    } else if (s == std::string("FALSE") || s == std::string("0")) {
        return false;
    } else {
        throw "unrecognized string in string_to_bool!";
    }
}

List<Create_field>
createAndRewriteField(Analysis &a, const ProxyState &ps,
                      Create_field *cf, TableMeta *tm,
                      bool new_table,
                      List<Create_field> &rewritten_cfield_list)
{
    // -----------------------------
    //         Update FIELD       
    // -----------------------------
    const std::string name = std::string(cf->field_name);
    FieldMeta * const fm =
        new FieldMeta(name, cf, ps.masterKey, tm->leaseIncUniq());
    // Here we store the key name for the first time. It will be applied
    // after the Delta is read out of the database.
    if (true == new_table) {
        tm->addChild(new IdentityMetaKey(name), fm);
    } else {
        // FIXME: dynamic_cast
        DDLOutput *ddl_output = static_cast<DDLOutput *>(a.output);
        const Delta d(Delta::CREATE, fm, tm, new IdentityMetaKey(name));
        ddl_output->addDelta(d);
        const Delta d0(Delta::REPLACE, tm, a.getSchema(),
                       a.getSchema()->getKey(tm));
        ddl_output->addDelta(d0);
    }

    // -----------------------------
    //         Rewrite FIELD       
    // -----------------------------
    auto new_fields = rewrite_create_field(fm, cf, a);
    rewritten_cfield_list.concat(vectorToList(new_fields));

    return rewritten_cfield_list;
}

//TODO: which encrypt/decrypt should handle null?
Item *
encrypt_item_layers(Item * i, onion o, OnionMeta * const om,
                    Analysis &a, uint64_t IV) {
    assert(!i->is_null());

    if (o == oPLAIN) {//Unencrypted item
        return i;
    }

    // Encrypted item

    const auto enc_layers = a.getEncLayers(om);
    assert_s(enc_layers.size() > 0, "field must have at least one layer");
    Item * enc = i;
    Item * prev_enc = NULL;
    for (auto layer : enc_layers) {
        LOG(encl) << "encrypt layer " << levelnames[(int)layer->level()] << "\n";
        enc = layer->encrypt(enc, IV);
        //need to free space for all enc
        //except the last one
        if (prev_enc) {
            delete prev_enc;
        }
        prev_enc = enc;
    }

    return enc;
}

