#pragma once

#include <crypto-old/CryptoManager.hh>
#include <util/onions.hh>

#include <parser/embedmysql.hh>
#include <parser/stringify.hh>
#include <main/MultiPrinc.hh>
#include <util/cryptdb_log.hh>

using namespace std;


/**
 * Used to keep track of encryption constraints during
 * analysis
 */
class EncSet {
public:
    EncSet(OnionLevelFieldMap input) : osl(input) {}
    EncSet(); // TODO(stephentu): move ctor here
    EncSet(FieldMeta * fm);
    EncSet(const OLK & olk);
    
    /**
     * decides which encryption scheme to use out of multiple in a set
     */
    OLK chooseOne() const;

    bool contains(const OLK & olk) const;
    
    EncSet intersect(const EncSet & es2) const;

    inline bool empty() const { return osl.empty(); }

    inline bool singleton() const { return osl.size() == 1; }

    EncDesc encdesc();
    
    inline OLK extract_singleton() const {
        assert_s(singleton(), string("encset has size ") + StringFromVal(osl.size()));
	auto it = osl.begin();
	return OLK(it->first, it->second.first, it->second.second);
    }

    void setFieldForOnion(onion o, FieldMeta * fm);
    
    OnionLevelFieldMap osl; //max level on each onion
};


ostream&
operator<<(ostream &out, const EncSet & es);

const EncSet EQ_EncSet = {
        {
	    {oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
            {oDET,   LevelFieldPair(SECLEVEL::DET, NULL)},
            {oOPE,   LevelFieldPair(SECLEVEL::OPE, NULL)},
        }
};

const EncSet ORD_EncSet = {
    {
	{oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
	{oOPE, LevelFieldPair(SECLEVEL::OPE, NULL)},
    }
};

const EncSet PLAIN_EncSet = {
    {
	{oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
    }
};

//todo: there should be a map of FULL_EncSets depending on item type
const EncSet FULL_EncSet = {
        {
	    {oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
            {oDET, LevelFieldPair(SECLEVEL::RND, NULL)},
            {oOPE, LevelFieldPair(SECLEVEL::RND, NULL)},
            {oAGG, LevelFieldPair(SECLEVEL::HOM, NULL)},
            {oSWP, LevelFieldPair(SECLEVEL::SEARCH, NULL)},
        }
};

const EncSet FULL_EncSet_Str = {
        {
	    {oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
            {oDET, LevelFieldPair(SECLEVEL::RND, NULL)},
            {oOPE, LevelFieldPair(SECLEVEL::RND, NULL)},
            {oSWP, LevelFieldPair(SECLEVEL::SEARCH, NULL)},
        }
};

const EncSet FULL_EncSet_Int = {
        {
	    {oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
            {oDET, LevelFieldPair(SECLEVEL::RND, NULL)},
            {oOPE, LevelFieldPair(SECLEVEL::RND, NULL)},
            {oAGG, LevelFieldPair(SECLEVEL::HOM, NULL)},
        }
};

const EncSet Search_EncSet = {
        {
	    {oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
            {oSWP, LevelFieldPair(SECLEVEL::SEARCH, NULL)},
        }
};

const EncSet ADD_EncSet = {
        {
	    {oPLAIN, LevelFieldPair(SECLEVEL::PLAINVAL, NULL)},
            {oAGG, LevelFieldPair(SECLEVEL::HOM, NULL)},
        }
};

const EncSet EMPTY_EncSet = {
        {{}}
};


// returns true if any of the layers in ed
// need salt
bool
needsSalt(EncSet ed);

/***************************************************/

extern "C" void *create_embedded_thd(int client_flag);

typedef struct ReturnField {//TODO: isn't FieldMeta more fit than ItemMeta?
    bool is_salt;
    std::string field_called;
    OLK olk;
    int pos_salt; //position of salt of this field in the query results,
                  // or -1 if such salt was not requested
    string stringify();
} ReturnField;

typedef struct ReturnMeta {
    map<int, ReturnField> rfmeta;
    string stringify();
} ReturnMeta;


class OnionAdjustExcept {
public:
    OnionAdjustExcept(onion o, FieldMeta * fm, SECLEVEL l, Item_field * itf) :
	o(o), fm(fm), tolevel(l), itf(itf) {}

    onion o;
    FieldMeta * fm;
    SECLEVEL tolevel;
    Item_field * itf;
};


class reason {
public:
    reason(const EncSet & es, const std::string &why_t_arg,
                Item *why_t_item_arg)
	:  encset(es), why_t(why_t_arg), why_t_item(why_t_item_arg)
	{ childr = new std::list<reason>();}
    reason(): encset(EMPTY_EncSet), why_t(""), why_t_item(NULL), childr(NULL) {}
    void add_child(const reason & ch) {
	childr->push_back(ch);
    }
    
    EncSet encset;    

    std::string why_t;
    Item *why_t_item;
    
    std::list<reason> * childr;
};

ostream&
operator<<(ostream &out, const reason &r);

class RewritePlan {
public:
    reason r;
    EncSet es_out; // encset that this item can output
    
    // plan for how to rewrite an item based on what the enc level the
    // parent asks this item for
    //olk needed by parent child   olk to ask to child
    std::map<OLK, std::map<Item *, OLK> > plan;

    RewritePlan() {};
    
    //constructor for a node with only one child and one outgoing olk
    RewritePlan(const OLK & parent_olk, const OLK & childr_olk,
		Item ** childr, uint no_childr, reason r);

    //constructor for a node with no children
    RewritePlan(const EncSet & es, reason r);
    
    //only keep plans that have parent_olk in es
    void restrict(const EncSet & es);

};


ostream&
operator<<(ostream &out, const RewritePlan * rp);

class Analysis {
public:
    Analysis(Connect * e_conn, Connect * conn, SchemaInfo * schema, AES_KEY *key, MultiPrinc *mp)
        : schema(schema), masterKey(key), conn(conn), e_conn(e_conn), mp(mp), pos(0) {}
    
    Analysis(): schema(NULL), masterKey(NULL), conn(NULL), e_conn(NULL), mp(NULL), pos(0) {}
 

    // Proxy Metadata    
    SchemaInfo *                        schema;
    AES_KEY *                           masterKey;
    
    Connect *                           conn;
    Connect *                           e_conn;

    MultiPrinc *                        mp;

    // Query specific information

    // maps an Item to ways we could rewrite the item and its children
    // does not contain Items that do not have children
    map<Item *, RewritePlan *> itemRewritePlans;
   
    unsigned int pos; //a counter indicating how many projection fields have been analyzed so far                                                                    
    std::map<FieldMeta *, salt_type>    salts;  
    ReturnMeta rmeta;

    TMKM                                tmkm; //for multi princ
private:
    MYSQL * m;

};

typedef struct ConnectionData {
    std::string server;
    std::string user;
    std::string psswd;
    std::string dbname;
    uint port;

    ConnectionData() {}

    ConnectionData(std::string serverarg, std::string userarg, std::string psswdarg, std::string dbnamearg, uint portarg = 0) {
        server = serverarg;
        user = userarg;
        psswd = psswdarg;
        dbname = dbnamearg;
        port = portarg;
    }

} ConnectionData;
