<?php

require("common.php");
//print_r($_POST);

 //get initial values
 $SQLq=trim($_REQUEST['q']);
 $page=$_REQUEST['p']+0;
 if ($_REQUEST['refresh'] && $DB['db'] && preg_match('/^show/',$SQLq) ) $SQLq=$SHOW_T;
 

 // Connects to CryptDB proxy
//cryptdb_connect('nodie');

 /*
  * Commands handler
  */
if (db_connect('nodie')){

    // Make sure mysql is available
    // $dbh is global.
    if(!$dbh)
        die("Fatal: Backend DB not connected.");

    // Going to fail here, proxy DB connection impl. is not finished yet.
    //if(!$cryptdbh)
    //    die("Fatal: Proxy DB not connected. (unfinished impl.)");

    $time_start=microtime_float();

    if ($_REQUEST['phpinfo']){
        ob_start();phpinfo();$sqldr='<div style="font-size:130%">'.ob_get_clean().'</div>';
    }else{
        if(isset($_SESSION['s_PROXY']) && !isset($DB['db']))
        {
            /*
             * Execute query (proxy db)
             */
            $num = -1;
            $type = "";
            // only first one is of interest
            foreach($_POST as $key=>$value)
            {
                $type = $value;
                $num = strtok($key, "_");
                if(!is_numeric($num))
                    die("Parse error");

                break;
            }
            if($num == -1)
                die("Parse error");

            //TODO(ccarvalho): parse s_ID and format  & execute query in CryptDB
            //do_sql($q);
            //echo "TYPE:" . $value . "<br>"; 
            //echo "Pos:" . $num . "<br>";
            //session_debug('s_PROXY');
            //session_debug('s_DB');
            //session_debug('s_QUERY');
            //session_debug('s_ID');

            session_remove('s_PROXY');
            session_remove('s_DB');
            session_remove('s_QUERY');
            session_remove('s_ID');

        } else if (isset($DB['db']))
        {

            /*
             * Describe_table
             *
             */
            if($_POST['cryptdb_describe_table'])
            {
                $q = "describe " . $DB['db'] . "." . $_POST['cryptdb_describe_table'];
                do_sql($q);

            }
        }else{
            if ( $_REQUEST['refresh'] ){
                check_xss();do_sql('show databases');
            }elseif ( preg_match('/^show\s+(?:databases|status|variables|process)/i',$SQLq) ){
                check_xss();do_sql($SQLq);
            }else{
                $err_msg="Select Database first";
                if (!$SQLq) do_sql("show databases");
            }
        }
    }
    $time_all=ceil((microtime_float()-$time_start)*10000)/10000;

    print_screen();
}else{
    print_cfg();
 }


function print_header(){
 global $err_msg,$VERSION,$DB,$CRYPTDB,$dbh,$self,$is_sht,$xurl,$SHOW_T;
 $dbn=$DB['db'];
?>

<!-- HTML BODY -->


<!DOCTYPE html>
<html>
<head><title>CryptDB</title>
<meta charset="utf-8">


<!-- CSS STYLE DEFINITIONS -->
<style type="text/css">
body{font-family:Arial,sans-serif;font-size:80%;padding:2;margin:2}
th,td{padding:0;margin:0}
div{padding:3px}
pre{font-size:50%}
.nav{text-align:center}
.ft{text-align:right;margin-top:10px;font-size:smaller}
.inv{background-color:#069;color:#FFF}
.inv a{color:#FFF}
table.res{width:50%;border-collapse:collapse;}
table.wa{width:auto}
table.res th,table.res td{padding:2px;border:1px solid #fff}
table.restr{vertical-align:top}
tr.e{background-color:#CCC}
tr.o{background-color:#EEE}
tr.h{background-color:#99C}
tr.s{background-color:#FF9}
.err{color:#F33;font-weight:bold;text-align:center}
.frm{width:400px;border:1px solid #999;background-color:#eee;text-align:left}
.frm label.l{width:100px;float:left}
.dot{border-bottom:1px dotted #000}
.ajax{text-decoration: none;border-bottom: 1px dashed;}
.qnav{width:30px}
</style>

<script type="text/javascript">
var LSK='pma_',LSKX=LSK+'max',LSKM=LSK+'min',qcur=0,LSMAX=32;

function $(i){return document.getElementById(i)}
function frefresh(){
    var F=document.DF;
    F.method='get';
    F.refresh.value="1";
    F.submit();
}
function go(p,sql){
    var F=document.DF;
    F.p.value=p;
    if(sql)F.q.value=sql;
    F.submit();
}
function ays(){
    return confirm('Are you sure to continue?');
}
function chksql(){
    var F=document.DF,v=F.q.value;
    if(/^\s*(?:delete|drop|truncate|alter)/.test(v)) if(!ays())return false;
    if(lschk(1)){
        var lsm=lsmax()+1,ls=localStorage;
        ls[LSK+lsm]=v;
        ls[LSKX]=lsm;
        //keep just last LSMAX queries in log
        if(!ls[LSKM])ls[LSKM]=1;
        var lsmin=parseInt(ls[LSKM]);
        if((lsm-lsmin+1)>LSMAX){
            lsclean(lsmin,lsm-LSMAX);
        }
    }
    return true;
}
function tmv(tr){
    tr.sc=tr.className;
    tr.className='h';
}
function tmo(tr){
    tr.className=tr.sc;
}
function tc(tr){
    tr.className='s';
    tr.sc='s';
}
function lschk(skip){
    if (!localStorage || !skip && !localStorage[LSKX]) return false;
    return true;
}
function lsmax(){
    var ls=localStorage;
    if(!lschk() || !ls[LSKX])return 0;
    return parseInt(ls[LSKX]);
}
function lsclean(from,to){
    ls=localStorage;
    for(var i=from;i<=to;i++){
        delete ls[LSK+i];ls[LSKM]=i+1;
    }
}
function after_load(){
    qcur=lsmax();
}
function logoff(){
    if(lschk()){
        var ls=localStorage;
        var from=parseInt(ls[LSKM]),to=parseInt(ls[LSKX]);
        for(var i=from;i<=to;i++){
            delete ls[LSK+i];
        }
        delete ls[LSKM];delete ls[LSKX];
    }
}

<?php if($is_sht){?>
function chkall(cab){
    var e=document.DF.elements;
    if (e!=null){
        var cl=e.length;
        for (i=0;i<cl;i++){
            var m=e[i];if(m.checked!=null && m.type=="checkbox"){
                m.checked=cab.checked
            }
        }
    }
}
function sht(f){
    document.DF.dosht.value=f;
}


<?php }?>
</script>

</head>

<?php
/*
 * Footer tags
 */
function print_footer(){
?>
</form>
</body></html>
<?php
}?>

<body onload="after_load()">
<form method="post" name="DF" action="<?php echo $self?>" enctype="multipart/form-data">
<input type="hidden" name="XSS" value="<?php echo $_SESSION['XSS']?>">
<input type="hidden" name="refresh" value="">
<input type="hidden" name="p" value="">


<!-- HEADER MENU -->
<div class="inv">
<?php if ($_SESSION['is_logged'] && $dbh){ ?>
Databases:
<select name="db" onChange="frefresh()">
<?php echo get_db_select($dbn)?>
</select>

<?php if($dbn){ $z=" &#183; <a href='$self?$xurl&db=$dbn"; ?>
<?php if($_POST['cryptdb_describe_table'] && $dbn) { 
    $z.'&q='.urlencode($var);
} ?>

Tables:
<form action="'<?php echo $self?>'" method="post">
<select name="cryptdb_describe_table" onChange="location.href='<?php $var = "describe " . $_POST['cryptdb_describe_table']; echo $z.'&q='.urlencode($var)?>'">
<?php 
 $a=0;
 $array = get_db_tables($dbn) ; foreach($array as $key=>$value) 
{ 
     $accesskey = "Tables_in_" . $dbn;
?>
<option value="<?php echo $value[$accesskey]; ?>"><?php echo $value[$accesskey]; ?></option>
<?php } ?>

</select>
<input type="submit" value="Describe table">
</form>
<?php } ?>
<?php } ?>




<?php if ($GLOBALS['ACCESS_PWD']){?> | <a href="?<?php echo $xurl?>&logoff=1" onclick="logoff()">Logoff</a> <?php }?>
</div>
<div class="err"><?php echo $err_msg?></div>

<?php
}

function print_screen(){
    global $out_message, $SQLq, $err_msg, $reccount, $time_all, $sqldr, $page, $MAX_ROWS_PER_PAGE, $is_limited_sql;

    $nav='';
    if ($is_limited_sql && ($page || $reccount>=$MAX_ROWS_PER_PAGE) ){
        $nav="<div class='nav'>".get_nav($page, 10000, $MAX_ROWS_PER_PAGE, "javascript:go(%p%)")."</div>";
    }

    print_header();
?>

<!--<textarea readonly id="q" name="q" cols="70" rows="5" style="width:50%"><?php echo $SQLq?></textarea><br>-->

Records: <b><?php echo $reccount?></b> in <b><?php echo $time_all?></b> sec<br>
<b><?php echo $out_message?></b>
<div class="sqldr">
<?php echo $nav.$sqldr.$nav; ?>
</div>
<?php
    print_footer();
}


function print_cfg(){
    global $DB,$CRYPTDB,$err_msg,$self;
    print_header();
?>
<center>
<h3>CryptDB</h3>
<div class="frm">

<label class="l">DB user:</label><input type="text" name="v[user]" value="<?php echo $DB['user']?>"><br>
<label class="l">Password:</label><input type="password" name="v[pwd]" value=""><br>
<br>

<label class="l">CryptDB user:</label><input type="text" name="c[user]" value="<?php echo $CRYPTDB['user']?>"><br>
<label class="l">Password:</label><input type="password" name="c[pwd]" value=""><br>
<label class="l">Port:</label><input type="text" name="c[port]" value="<?php echo $CRYPTDB['port']?>" size="4"><br>

<div id="cfg-adv" style="display:none;">



<label class="l">Charset:</label>

<select name="v[chset]"><option value="">- default -</option><?php echo chset_select($DB['chset'])?>
</select><br>

<br>


<br><input type="checkbox" name="rmb" value="1" checked> Remember in cookies for 30 days
</div>
<center>
<input type="hidden" name="savecfg" value="1">
<input type="submit" value=" Apply "><input type="button" value=" Cancel " onclick="window.location='<?php echo $self?>'">
</center>
</div>
</center>
<?php
    print_footer();
}


/*
 * Connection
 */
function db_connect($nodie=0){
    global $dbh,$DB,$err_msg;

    $dbh=@mysql_connect($DB['host'].($DB['port']?":$DB[port]":''),$DB['user'],$DB['pwd']);
    if (!$dbh) {
        $err_msg='Cannot connect to the database because: '.mysql_error();
        if (!$nodie) die($err_msg);
    }

    if ($dbh && $DB['db']) {
        $res=mysql_select_db($DB['db'], $dbh);
        if (!$res) {
            $err_msg='Cannot select db because: '.mysql_error();
            if (!$nodie) die($err_msg);
        }else{
            if ($DB['chset']) db_query("SET NAMES ".$DB['chset']);
        }
    }

    return $dbh;
}

function cryptdb_connect($nodie=0){
    global $cryptdbh,$CRYPTDB,$err_msg;

    //TODO: Test this implementation
    return;


    $cryptdbh=@mysql_connect($CRYPTDB['host'].($CRYPTDB['port']?":$CRYPTDB[port]":''),$CRYPTDB['user'],$CRYPTDB['pwd']);
    if (!$cryptdbh) 
    {
        $err_msg='Cannot connect to the CryptDB proxy database because: '.mysql_error();
        echo $err_msg;
        //TODO:FIX this
        die($err_msg);
    }

    if ($cryptdbh && $CRYPTDB['db']) {
        $res=mysql_select_db($CRYPTDB['db'], $cryptdbh);
        if (!$res) {
            $err_msg='Cannot select cryptdb because: '.mysql_error();
            if (!$nodie) 
            {
                die($err_msg);
            }
        }else{
            if ($CRYPTDB['chset']) db_query("SET NAMES ".$CRYPTDB['chset']);
        }
    }

    return $cryptdbh;
}

function db_checkconnect($dbh1=NULL, $skiperr=0){
    global $dbh;
    if (!$dbh1) $dbh1=&$dbh;
    if (!$dbh1 or !mysql_ping($dbh1)) {
        db_connect($skiperr);
        $dbh1=&$dbh;
    }
    return $dbh1;
}

function cryptdb_checkconnect($cryptdbh1=NULL, $skiperr=0){
    global $cryptdbh;
    if (!$cryptdbh1) $cryptdbh1=&$cryptdbh;
    if (!$cryptdbh1 or !mysql_ping($cryptdbh1)) {
        cryptdb_connect($skiperr);
        $cryptdbh1=&$cryptdbh;
    }
    return $cryptdbh1;
}

function db_disconnect(){
    global $dbh;
    mysql_close($dbh);
}

function dbq($s){
    global $dbh;
    if (is_null($s)) return "NULL";
    return "'".mysql_real_escape_string($s,$dbh)."'";
}

function db_query($sql, $dbh1=NULL, $skiperr=0){
    $dbh1=db_checkconnect($dbh1, $skiperr);
    $sth=@mysql_query($sql, $dbh1);
    if (!$sth && $skiperr) return;
    if (!$sth) die("Error in DB operation:<br>\n".mysql_error($dbh1)."<br>\n$sql");
    return $sth;
}

function db_array($sql, $dbh1=NULL, $skiperr=0, $isnum=0){#array of rows
    $sth=db_query($sql, $dbh1, $skiperr);
    if (!$sth) return;
    $res=array();
    if ($isnum){
        while($row=mysql_fetch_row($sth)) $res[]=$row;
    }else{
        while($row=mysql_fetch_assoc($sth)) $res[]=$row;
    }
    return $res;
}

function db_row($sql){
    $sth=db_query($sql);
    return mysql_fetch_assoc($sth);
}


function get_identity($dbh1=NULL){
    $dbh1=db_checkconnect($dbh1);
    return mysql_insert_id($dbh1);
}

function get_db_select($sel=''){
    global $DB;
    if (is_array($_SESSION['sql_sd']) && $_REQUEST['db']!='*'){//check cache
        $arr=$_SESSION['sql_sd'];
    }else{
        $arr=db_array("show databases",NULL,1);
        if (!is_array($arr)){
            $arr=array( 0 => array('Database' => $DB['db']) );
        }
        $_SESSION['sql_sd']=$arr;
    }
    return @sel($arr,'Database',$sel);
}

function get_db_tables($sel='') {
    $query = "show tables from " . $_REQUEST['db'];
    return ($arr=db_array($query, NULL, 1));
}


function sel($arr,$n,$sel=''){
    foreach($arr as $a){
        #   echo $a[0];
        $b=$a[$n];
        $res.="<option value='$b' ".($sel && $sel==$b?'selected':'').">$b</option>";
    }
    return $res;
}

function microtime_float(){
    list($usec,$sec)=explode(" ",microtime());
    return ((float)$usec+(float)$sec);
}

/* page nav
 $pg=int($_[0]);     #current page
 $all=int($_[1]);     #total number of items
 $PP=$_[2];      #number if items Per Page
 $ptpl=$_[3];      #page url /ukr/dollar/notes.php?page=    for notes.php
 $show_all=$_[5];           #print Totals?
*/
function get_nav($pg, $all, $PP, $ptpl, $show_all=''){
        $n='&nbsp;';
        $sep=" $n|$n\n";
        if (!$PP) $PP=10;
        $allp=floor($all/$PP+0.999999);

        $pname='';
        $res='';
        $w=array('Less','More','Back','Next','First','Total');

        $sp=$pg-2;
        if($sp<0) $sp=0;
        if($allp-$sp<5 && $allp>=5) $sp=$allp-5;

        $res="";

        if($sp>0){
            $pname=pen($sp-1,$ptpl);
            $res.="<a href='$pname'>$w[0]</a>";
            $res.=$sep;
        }
        for($p_p=$sp;$p_p<$allp && $p_p<$sp+5;$p_p++){
            $first_s=$p_p*$PP+1;
            $last_s=($p_p+1)*$PP;
            $pname=pen($p_p,$ptpl);
            if($last_s>$all){
                $last_s=$all;
            }
            if($p_p==$pg){
                $res.="<b>$first_s..$last_s</b>";
            }else{
                $res.="<a href='$pname'>$first_s..$last_s</a>";
            }
            if($p_p+1<$allp) $res.=$sep;
        }
        if($sp+5<$allp){
            $pname=pen($sp+5,$ptpl);
            $res.="<a href='$pname'>$w[1]</a>";
        }
        $res.=" <br>\n";

        if($pg>0){
            $pname=pen($pg-1,$ptpl);
            $res.="<a href='$pname'>$w[2]</a> $n|$n ";
            $pname=pen(0,$ptpl);
            $res.="<a href='$pname'>$w[4]</a>";
        }
        if($pg>0 && $pg+1<$allp) $res.=$sep;
        if($pg+1<$allp){
            $pname=pen($pg+1,$ptpl);
            $res.="<a href='$pname'>$w[3]</a>";
        }
        if ($show_all) $res.=" <b>($w[5] - $all)</b> ";

        return $res;
    }

function pen($p,$np=''){
    return str_replace('%p%',$p, $np);
}


//each time - from session to $DB_*
function loadsess(){
    global $DB, $CRYPTDB;

    $DB=$_SESSION['DB'];

    $rdb=$_REQUEST['db'];
    if ($rdb=='*') $rdb='';
    if ($rdb) {
        $DB['db']=$rdb;
    }

    $CRYPTDB=$_SESSION['CRYPTDB'];

    $cryptrdb=$_REQUEST['db'];
    if ($cryptrdb=='*') $cryptrdb='';
    if ($cryptrdb) {
        $CRYPTDB['db']=$cryptrdb;
    }
}


// multiple SQL statements splitter
function do_multi_sql($insql,$fname=''){
    set_time_limit(600);

    $sql='';
    $ochar='';
    $is_cmt='';
    $GLOBALS['insql_done']=0;
    while ($str=get_next_chunk($insql,$fname)){
        $opos=-strlen($ochar);
        $cur_pos=0;
        $i=strlen($str);
        while ($i--){
            if ($ochar){
                list($clchar, $clpos)=get_close_char($str, $opos+strlen($ochar), $ochar);
                if ( $clchar ) {
                    if ($ochar=='--' || $ochar=='#' || $is_cmt ){
                        $sql.=substr($str, $cur_pos, $opos-$cur_pos );
                    }else{
                        $sql.=substr($str, $cur_pos, $clpos+strlen($clchar)-$cur_pos );
                    }
                    $cur_pos=$clpos+strlen($clchar);
                    $ochar='';
                    $opos=0;
                }else{
                    $sql.=substr($str, $cur_pos);
                    break;
                }
            }else{
                list($ochar, $opos)=get_open_char($str, $cur_pos);
                if ($ochar==';'){
                    $sql.=substr($str, $cur_pos, $opos-$cur_pos+1);
                    if (!do_one_sql($sql)) return 0;
                    $sql='';
                    $cur_pos=$opos+strlen($ochar);
                    $ochar='';
                    $opos=0;
                }elseif(!$ochar) {
                    $sql.=substr($str, $cur_pos);
                    break;
                }else{
                    $is_cmt=0;if ($ochar=='/*' && substr($str, $opos, 3)!='/*!') $is_cmt=1;
                }
            }
        }
    }

    if ($sql){
        if (!do_one_sql($sql)) return 0;
        $sql='';
    }
    return 1;
}


?>