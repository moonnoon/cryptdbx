#include <stdio.h>
#include <bsd/string.h>

#include "sql_priv.h"
#include "unireg.h"
#include "strfunc.h"
#include "sql_class.h"
#include "set_var.h"
#include "sql_base.h"
#include "rpl_handler.h"
#include "sql_parse.h"
#include "sql_plugin.h"
#include "derror.h"

static void
parse(const char *q)
{
    my_thread_init();
    THD *t = new THD;
    if (t->store_globals())
        printf("store_globals error\n");

    if (init_errmessage())
        printf("init_errmessage error\n");

    char buf[1024];
    strlcpy(buf, q, sizeof(buf));
    size_t len = strlen(buf);

    alloc_query(t, buf, len + 1);

    Parser_state ps;
    if (!ps.init(t, buf, len)) {
        LEX lex;
        t->lex = &lex;

        lex_start(t);
        mysql_reset_thd_for_next_command(t);

        t->set_db("", 0);

        printf("q=%s\n", buf);
        bool error = parse_sql(t, &ps, 0);
        if (error) {
            printf("parse error: %d %d %d\n", error, t->is_fatal_error,
                   t->is_error());
            printf("parse error: h %p\n", t->get_internal_handler());
            printf("parse error: %d %s\n", t->is_error(), t->get_stmt_da()->message());
        } else {
            printf("command %d\n", lex.sql_command);

            String s;
            lex.select_lex.print(t, &s, QT_ORDINARY);
            //lex.unit.print(&s, QT_ORDINARY);
            printf("reconstructed query: %s\n", s.c_ptr());
        }

        t->end_statement();
    } else {
        printf("parser init error\n");
    }

    t->cleanup_after_query();
    delete t;
}

int
main(int ac, char **av)
{
    system_charset_info = &my_charset_utf8_general_ci;
    global_system_variables.character_set_client = system_charset_info;
    table_alias_charset = &my_charset_bin;

    pthread_key_t dummy;
    if (pthread_key_create(&dummy, 0) ||
        pthread_key_create(&THR_THD, 0) ||
        pthread_key_create(&THR_MALLOC, 0))
        printf("pthread_key_create error\n");

    sys_var_init();
    lex_init();
    item_create_init();
    item_init();

    my_init();
    mdl_init();
    table_def_init();
    randominit(&sql_rand, 0, 0);
    delegates_init();
    init_tmpdir(&mysql_tmpdir_list, 0);

    default_charset_info =
        get_charset_by_csname("utf8", MY_CS_PRIMARY, MYF(MY_WME));
    global_system_variables.collation_server         = default_charset_info;
    global_system_variables.collation_database       = default_charset_info;
    global_system_variables.collation_connection     = default_charset_info;
    global_system_variables.character_set_results    = default_charset_info;
    global_system_variables.character_set_client     = default_charset_info;
    global_system_variables.character_set_filesystem = default_charset_info;

    my_default_lc_messages = my_locale_by_name("en_US");
    global_system_variables.lc_messages = my_default_lc_messages;

    opt_ignore_builtin_innodb = true;
    int plugin_ac = 1;
    char *plugin_av = (char *) "x";
    plugin_init(&plugin_ac, &plugin_av, 0);

    //const char *engine = "MEMORY";
    //LEX_STRING name = { (char *) engine, strlen(engine) };
    //plugin_ref plugin = ha_resolve_by_name(0, &name);
    //global_system_variables.table_plugin = plugin;

    if (ac != 2) {
        printf("Usage: %s query\n", av[0]);
        exit(1);
    }

    const char *q = av[1];
    parse(q);
}
