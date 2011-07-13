#include "openssl/rand.h"

#include "util.h"



using namespace std;


void myassert(bool value, const char * mess) {
	if (ASSERTS_ON) {
		if (!value) {
			fprintf(stderr, "ERROR: %s\n", mess);
			throw mess;
		}
	}
}

void myassert(bool value, string mess) {
	if (ASSERTS_ON) {
		if (!value) {
			fprintf(stderr, "ERROR: %s\n", mess.c_str());
			throw mess.c_str();
		}
	}
}

void myassert(bool value) {
	if (ASSERTS_ON) {
		if (!value) {
			throw "Assertion failed!";
		}
	}
}


void assert_s (bool value, string msg) throw (SyntaxError) {
	if (ASSERTS_ON) {
		if (!value) {
			SyntaxError se;
			se.msg = "ERROR: " + msg + "\n";
			cerr << se.msg << "\n";
			throw se;
		}
	}
}

ParserMeta::ParserMeta() {

	const unsigned int noKeywords = 32;
	const string clauseKeywords[] = {"select", "from",  "where",  "order", "group",  "update", "set", "insert",
			"into", "and",  "or",  "distinct",   "in", "*", "max", "min", "count", "sum", "by", "asc",
			"desc", "limit",
			"null",
			"ilike", "like",
			"integer", "bigint", "text",
			"left", "join", "on", "is"};


	const unsigned int noSeparators = 13;
	const string querySeparators[] = {"from", "where", "left", "on", "group",
			"order", "limit","values", ";", "set", "group",  "asc", "desc"};

	clauseKeywords_p = std::set<string>();
	for (unsigned int i = 0; i < noKeywords; i++) {
		clauseKeywords_p.insert(clauseKeywords[i]);
	}
	querySeparators_p = std::set<string>();
	for (unsigned int i = 0; i < noSeparators; i++) {
		querySeparators_p.insert(querySeparators[i]);
	}

}

double timeInSec(struct timeval tvstart, struct timeval tvend) {
	double interval = ((tvend.tv_sec - tvstart.tv_sec)*1.0 + (tvend.tv_usec - tvstart.tv_usec)/1000000.0);
    return interval;
}

double timeInMSec(struct timeval tvstart, struct timeval tvend) {
	return timeInSec(tvstart, tvend) * 1000.0;
}


unsigned char * concatenate(unsigned char * a, unsigned int aLen, unsigned char * b, unsigned int bLen, unsigned char * c, unsigned int cLen) {
	unsigned int len = aLen + bLen + cLen;
	unsigned char * result = new unsigned char[len];

	for (unsigned int i = 0; i < aLen; i++) {
		result[i] = a[i];
	}

	for (unsigned int i = aLen; i < aLen + bLen; i++) {
		result[i] =  b[i-aLen];
	}

	for (unsigned int i = aLen + bLen; i < len; i++) {
		result[i] =  c[i-aLen-bLen];
	}

	return result;
}

unsigned char * randomBytes(unsigned int len) {
	unsigned char * res = new unsigned char[len+4];
	RAND_bytes(res, len);

	return res;
}

uint64_t randomValue() {
	return IntFromBytes(randomBytes(8), 8);
}

void myPrint(unsigned char * a, unsigned int aLen) {
	for (unsigned int i = 0; i < aLen; i++) {
		fprintf(stderr, "%d ", (int)(a[i]-0));
	}
}

void myPrint(vector<bool> & bitmap) {
	for (vector<bool>::iterator it = bitmap.begin(); it != bitmap.end(); it++) {
		cout << *it << " ";
	}
	cout << "\n";
}

void myPrint(unsigned int * a, unsigned int aLen) {
	for (unsigned int i = 0; i < aLen; i++) {
		fprintf(stderr, "%d ", a[i]);
	}
}

void myPrint(char * a) {
	for (unsigned int i = 0; i<strlen(a); i++) {
		fprintf(stderr, "%d ", a[i]);

	}
}


string checkStr(list<string>::iterator & it, list<string> & words, string s1, string s2) {

	if (it == words.end()) {
		return "";
	}
	if (it->compare(s1) == 0) {
		it++;
		return s1;
	}
	if (it->compare(s2) == 0) {
		return "";
	}
	if (isQuerySeparator(*it)) {
		return "";
	}

	assert_s(false, string("expected ") + s1 + " or " + s2 + " given " + *it );
	return "";
}

void myPrint(const list<string> & lst) {
	for (list<string>::const_iterator it = lst.begin(); it!=lst.end(); it++) {
		fprintf(stderr, " %s,", it->c_str());
	}

	fprintf(stderr, "\n");
}

string toString(const list<string> & lst) {
	list<string>::const_iterator it;

	string res = "";

	for (it = lst.begin(); it!=lst.end(); it++) {
		res = res + "<" + *it + "> ";
	}

	return res;
}


string toString(const vector<bool> & vec) {
	vector<bool>::const_iterator it;

	string res = "";

	for (it = vec.begin(); it!=vec.end(); it++) {
		res = res + StringFromVal(*it) + " ";
	}

	return res;
}

string toString(const std::set<string> & lst) {
	std::set<string>::const_iterator it;

	string res = "";

	for (it = lst.begin(); it != lst.end(); it++) {
		res = res + *it + " ";
	}

	return res;
}


void myPrint(const vector<vector<string> > & d) {
	unsigned int rows = d.size();
	if (d.size() == 0) {return;}
	unsigned int cols = d[0].size();
	for (unsigned int i = 0; i < rows; i++) {
		for (unsigned int j = 0; j < cols; j++) {
			fprintf(stderr, " %s15", getCStr(d[i][j]));
		}
		cerr << "\n";
	}
	cerr << "\n";
 }


string toString(const ResType & rt) {
	unsigned int n = rt.size();

	string res = "";

	for (unsigned int i = 0; i < n; i++) {
		unsigned int m = rt[i].size();
		for (unsigned int j = 0; j < m; j++) {
			res = res + " " + getCStr(rt[i][j]);
		}
		res += "\n";
	}

	return res;
}


void myPrint(list<const char *> & lst) {
	for (list<const char *>::iterator it = lst.begin(); it!=lst.end(); it++) {
		fprintf(stderr, " %s \n", *it);
	}

}

void myPrint(const vector<string> & lst) {
	for (vector<string>::const_iterator it = lst.begin(); it!=lst.end(); it++) {
		fprintf(stderr, " %s \n", it->c_str());
	}

}

char * getCStr(string value) {
	unsigned int len = value.length();
	char * result = new char[len+1];
	strncpy(result, value.c_str(), len);
	result[len] = '\0';

	return result;
}

bool isEqual(unsigned char * first, unsigned char * second, unsigned int len) {
	for (unsigned int i = 0; i < len; i++) {
		if (second[i] != first[i]) {
			return false;
		}
	}

	return true;
}

unsigned char * adjustLen(unsigned char * a, unsigned int len, unsigned int desiredLen) {
	if (len == desiredLen) {
		return a;
	}

	unsigned char * result = new unsigned char[desiredLen];

	if (len > desiredLen) {

		unsigned int offset = len-desiredLen;
		for (unsigned int i = 0; i < desiredLen; i++) {
			result[i] = a[i+offset];
		}

		return result;
	}

	//len < desiredLen : must pad
	for (unsigned int i = 0; i < len; i++) {
		result[i] = a[i];
	}
	for (unsigned int i = len; i < desiredLen; i++) {
		result[i] = 0;
	}

	return result;

}


unsigned char * BytesFromInt(uint64_t value, unsigned int noBytes) {
  unsigned char * result = (unsigned char *) malloc (noBytes+4);
	for (unsigned int i = 0 ; i < noBytes; i++) {
		result[noBytes-i-1] = value % 256;
		value = value / 256;
	}

	return result;
}

uint64_t IntFromBytes(unsigned char * bytes, unsigned int noBytes) {
	uint64_t value = 0;

	for (unsigned int i = 0; i < noBytes; i++) {
		unsigned int bval = (unsigned int)bytes[i];
		value = value * 256 + bval;
	}

    return value;
}

unsigned char* BytesFromZZ(const ZZ & x, unsigned int noBytes){
	unsigned char * result = new unsigned char[noBytes];
	BytesFromZZ(result, x, noBytes);
	return result;
};

string StringFromVal(unsigned int value, unsigned int desiredLen) {
	string result = "";

	for (unsigned int i = 0; i < desiredLen; i++) {
		char c = '0' + (value % 10);
		result = c + result;
		value = value/10;
	}

	return result;
};


string StringFromVal(unsigned long value) {
	string res = "";

	if (value == 0) {return "0";}

	while (value > 0) {
		char c = '0' + (value % 10);
		res = c + res;
		value = value / 10;
	}

	return res;
}

ZZ UInt64_tToZZ (uint64_t value) {
	unsigned int unit = 256;
	ZZ power;
	power = 1;
	ZZ res;
	res = 0;

	while (value > 0) {
		res = res + (value % unit) * power;
		power = power * unit;
		value = value / unit;
	}
	return res;

};



unsigned char ByteFromCharA(const char * a) {
	unsigned int len = strlen(a);
	int value = 0;
	for (unsigned int i = 0; i < len; i++) {
		myassert( (a[i]>='0') && (a[i] <='9'), "bad input");
		value =  value * 10 + (int)(a[i]-'0');
	}

	return (unsigned char) value;
}





//copies into res at position pos and len bytes data from data
unsigned char * copyInto(unsigned char * res, unsigned char * data, unsigned int pos, unsigned int len) {
	for (unsigned int i = 0; i< len; i++) {
		res[pos+i] = data[i];
	}
	return res;
}

/******
 *
 * Postgres accepts char * as queries, whereas encrypted values are unsigned char *.
 * The best way to convert between the two (with minimum storage overhead is below)
 *
 *******/
unsigned char * CharToUChar(const char * s) {
	ZZ value = to_ZZ(0);
	unsigned len = strlen(s);


	for (unsigned int i = 0; i < len; i++) {
		value = value * 255 + (s[i]-1);
	}

	return BytesFromZZ(value, len-1);


}

char * UCharToChar(unsigned char * s, unsigned int len) {
	ZZ value = ZZFromBytes(s, len);

	unsigned int charLen = len + 1;
	char * result = new char[len];

	for (unsigned int i = 0; i <  charLen; i++) {
		result[charLen-i-1] = to_int(value % 255 + 1);
		value = value / 255;
	}

	return result;
}

unsigned char * stringToUChar(string s, unsigned int len) {
	if (s.length() > len) {
		s = s.substr(0, len);
	} else {
	  unsigned int str_length = s.length();
	  for(unsigned int i = str_length; i < len; i++) {
			s = s + '0';
		}
	}
	unsigned char * res = new unsigned char[len];

	for (unsigned int i = 0; i < len; i++) {
		res[i] = s[i];
	}
	return res;
}

unsigned char *stringToUCharDirect(string input) {
	unsigned int len = input.length();
	unsigned char * result = new unsigned char[len + 1];
	result[len] = '\0';

	for (unsigned int i = 0; i < len; i++) {
		result[i] = input[i];
	}

	return result;
}

string uCharToStringDirect(unsigned char * input, unsigned int len) {
	string res = "";

	for (unsigned int i = 0; i < len; i++) {
		res = res + (char)input[i];
	}

	return res;
}

//returns a string representing a value pointed to by it and advances it
//skips apostrophes if there are nay
string getVal(list<string>::iterator & it) {
	string res;

	if (hasApostrophe(*it)) {
		res = removeApostrophe(*it);
	} else {
		res = *it;
	}
	it++;

	return res;

}

list<string> makeList(string val1, string val2) {
	list<string> res = list<string>();
	res.push_back(val1);
	res.push_back(val2);
	return res;
}
list<int> makeList(int val1, int val2) {
	list<int> res = list<int>();
	res.push_back(val1);
	res.push_back(val2);
	return res;
}

string marshallVal(uint64_t x) {

	int64_t xx = (int64_t) x;
	string res = "";
	bool sign = false;

	if (xx == 0) {return "0";}
	if (xx < 0) {
		sign = true;
		xx = xx * (-1);
	}


	while (xx > 0) {
		res =  (char)((unsigned char)'0'+(unsigned char)(xx % 10)) + res;
		xx = xx / 10;
	}
	if (sign) {
		res = "-" + res;
	}


	return res;

}

string marshallVal(unsigned int x, unsigned int digits) {
	string res = marshallVal(x);
	int delta = digits - res.length();

	for (int i = 0; i < delta; i++) {
		res = "0" + res;
	}
	return res;
}

string marshallVal(unsigned int x) {

	int32_t xx = (int32_t) x;
	string res = "";
	bool sign = false;

	if (xx == 0) {return "0";}
	if (xx < 0) {
		sign = true;
		xx = xx * (-1);
	}


	while (xx > 0) {
		res =  (char)((unsigned char)'0'+(unsigned char)(xx % 10)) + res;
		xx = xx / 10;
	}
	if (sign) {
		res = "-" + res;
	}


	return res;

}


uint64_t unmarshallVal(string str) {
	bool sign = false;
	if (str[0] == '-') {
		sign = true;
	}

	int64_t val = 0;

	unsigned int len = str.length();

	const char * cstr = str.c_str();

	unsigned int i;
	if (sign) {
		i = 1;
	} else {
		i = 0;
	}

	for (; i < len; i++) {
		myassert((cstr[i] <= '9') && (cstr[i] >= '0'), "invalid string " + str + " to be transformed in value ");

		val = val * 10 + ((unsigned char)cstr[i]-(unsigned char)'0');

	}

	if (sign) {
		val = val * (-1);
	}

	uint64_t vall = (uint64_t) val;

	return vall;
}

string toThreeDigits(unsigned int c) {
	char s[4];
	sprintf(s, "%03u", c);

	return string(s);
}

char * toHex(unsigned int abyte) {
	char * res =  new char[3];
	sprintf(res, "%02x", abyte);
	return res;

}




string octalRepr(int c) {
	string res = "";
	char aux = '0' + (c % 8);
	res = aux + res;
	c = c/8;
	aux = '0'+ (c % 8);
	res = aux + res;
	c = c/8;
	aux = '0'+ (c % 8);
	res = aux + res;
	return res;
}
string secondMarshallBinary(unsigned char * v, unsigned int len) {


	cerr << "\n \n WRONG BINARY \n \n";
	cout << "\n \n WRONG BINARY \n \n"; fflush(stdout);

	string res = "E'";
	cerr << "calling wrong marshall \n";
	for (unsigned int i = 0; i < len; i++) {
		int c = (int) v[i];
		if (c == 39) {
			res = res + "\\\\" + "047"; //StringFromInt((int)v[i], 3)
			continue;
		}
		if (c == 92) {
			res = res + "\\\\" + "134";
			continue;
		}
		if ((c >=0 && c <= 31) || (c >= 127 && c<=255)) {
			res = res + "\\\\" + octalRepr(c);
			continue;
		}
		res = res + (char)v[i];
	}
	res = res + "'";
	return res;
}

#if MYSQL_S



string marshallBinary(unsigned char * binValue, unsigned int len) {

	string result = "X\'";

	for (unsigned int i = 0; i < len; i++) {
		result = result + toHex((unsigned int)binValue[i]);
	}

	result = result + "\'";

	//cerr << "output from marshall  " << result.c_str() << "\n";
	return result;

}

#else
string marshallBinary(unsigned char * binValue, unsigned int len) {

	return secondMarshallBinary(binValue, len);

}
#endif

unsigned char getFromHex(char * hexValues) {
	unsigned int v;
	sscanf(hexValues, "%2x", &v);
	return (unsigned char) v;

}

unsigned char * unmarshallBinary(char * value, unsigned int len, unsigned int & newlen) {
	unsigned int offset;
	//cerr << "input to unmarshall " << value << "\n";
#if MYSQL_S
	offset = 2;
	myassert(value[0] == 'X', "unmarshallBinary: first char is not x; it is " + value[0]);
	len = len - 1; //removing last apostrophe
#else
	myassert(value[0] == '\\', "unmarshallBinary: first char is not slash; it is " + value[0]);
	myassert(value[1] == 'x', "unmarshallBinary: second char is not x; it is " + value[1]);
	offset = 2;
#endif

	myassert((len - offset) % 2 == 0, "unmarshallBinary: newlen is odd! newlen is " + marshallVal(len-offset));

	newlen = (len-offset)/2;
	unsigned char * result = new unsigned char[newlen];

	for (unsigned int i = 0; i < newlen; i++) {
		result[i] = getFromHex(value+i*2+offset);
	}

	return result;
}

char * copy(const char * v) {
	char * result = new char[strlen(v)+1];

	strcpy(result, v);
	result[strlen(v)] = '\0';

	return result;
}

unsigned char * copy(unsigned char * v, unsigned int len) {
	unsigned char * res = new unsigned char[len];
	memcpy(res, v, len);
	return res;
}

bool matches(const char * query, const char * delims, bool ignoreOnEscape = false, int index = 0) {

	bool res = (strchr(delims, query[0]) != NULL);

	if (res && (index > 0)) {
		char c = *(query-1);
		if (c =='\\') {
			return false;
		}
	}

	return res;
}

list<string> parse(const char * query, const char * delimsStay, const char * delimsGo, const char * keepIntact) {
	list<string> res = list<string>();
		unsigned int len = strlen(query);

		unsigned int index = 0;

		string word = "";

		while (index < len) {
			while ((index < len) && matches(query + index, delimsGo)) {
				index = index + 1;
			}

			while ((index < len) && matches(query+index, delimsStay)) {
				string sep = "";
				sep = sep + query[index];
				res.push_back(sep);
				index = index + 1;
			}

			if (index >= len) {break;}

			if (matches(query+index, keepIntact, true, index)) {

				word = query[index];



				index++;

				while (index < len)  {

					if (matches(query+index, keepIntact, true, index)) {
						break;
					}

					word = word + query[index];
					index++;
				}

				string msg = "keepIntact was not closed in <";
				msg = msg + query + "> at index " + marshallVal(index);
				assert_s((index < len)  &&  matches(query+index, keepIntact, index), msg);
				word = word + query[index];
				res.push_back(word);

				index++;

			}

			if (index >= len) {break;}

			word = "";
			while ((index < len) && (!matches(query+index, delimsStay)) && (!matches(query+index, delimsGo)) && (!matches(query+index, keepIntact))) {
				word = word + query[index];
				index++;
			}

			if (word.length() > 0) {res.push_back(word);}
		}


		return res;

}

void consolidateComparisons(list<string> & words) {
	list<string>::iterator it = words.begin();
	list<string>::iterator oldit;


	while (it!=words.end()) {
		//consolidates comparisons
		if (contains(*it, comparisons, noComps)) {
			string res = "";
			//consolidates comparisons
			while ((it != words.end()) && (contains(*it, comparisons, noComps))) {
				res += *it;
				oldit = it;
				it++;
				words.erase(oldit);
			}
			words.insert(it, res);
			it++;
			continue;
		}


		/*
			//consolidates negative numbers
			if (it->compare("-") == 0) {
				it++;
				if (isNumber(*it)) {
					string res = "-" + *it;
					it--;
					list<string>::iterator newit = it;
					newit--;
					words.insert(res, *it);
					words.erase(*it);
					it = ++newit;
					newit--;
					words.erase(it);
					newit++;
					it = newit;
				}
				continue;
			}
		 */
		it++;
	}

}

/*void evaluateMath(list<string> & words, list<string>::iterator start, list<string>::iterator end) {
  list<string>::iterator it = start;
  string res;
  //reduce the equations chosen
  while ((it != words.end()) && it != end) {
    assert_s(isOnly(*it, math, noMath), "input to evaluateMath is not math");
    res += *it;
    oldit = it;
    it++;	
    words.erase(oldit);
  }
  Equation eq;
  eq.set(res);
  res = eq.rpn();
  if (res.compare("") != 0) {
    words.insert(it, res);
  }
}

void consolidateMathSelect(list<string> & words) {

}

void consolidateMathInsert(list<string> & words) {

}

void consolidateMathUpdate(list<string> & words) {

}*/

void consolidateMath(list<string> & words) {
  command com = getCommand(getCStr(*words.begin()));
  switch (com) {
  case CREATE:
    cerr << "consolidateMath doesn't deal with CREATE" << endl;
    return;
  case UPDATE:
    //consolidateMathUpdate(words);
  case SELECT:
    //consolidateMathSelect(words);
  case INSERT:
    //consolidateMathInsert(words);
  case DROP:
    cerr << "consolidateMath doesn't deal with DROP" << endl;
    return;
  case DELETE:
    cerr << "consolidateMath doesn't deal with DELETE" << endl;
    return;
  case BEGIN:
    cerr << "consolidateMath doesn't deal with BEGIN" << endl;
    return;
  case COMMIT:
    cerr << "consolidateMath doesn't deal with COMMIT" << endl;
    return;
  case ALTER:
    cerr << "consolidateMath doesn't deal with ALTER" << endl;
    return;
  case OTHER:
    cerr << "consolidateMath doesn't deal with OTHER (what is this?)" << endl;
    return;
  }
}




/*
	list<string>::iterator it = words.begin();
	list<string>::iterator oldit = it;

	// This will only perform mathematical operations on equations where there are no variables in the words of the equations
	//   That is, it will simplify {"x=", "2+3"}, but not {"x=2","+3"}
	while (it != words.end()) {
	  //getCommand(first word)
	  list<string> select;
	  select.push_back("select");
	  list<string> insertinto;
	  insertinto.push_back("insert");
	  list<string> update;
	  update.push_back("update");
	  list<string> values;
	  values.push_back("values");
	  list<string> set;
	  set.push_back("set");
	  list<string> comma;
	  set.push_back(",");
	  set.push_back(")");
	  //for statements of the form SELECT ... WHERE ... = math
	  //use processWhere because UPDATE and DELETE are the same --- need to process things after WHERE
	  //Raluca will email function that sends it afer WHERE
	  if (it != words.end() && contains(*it, select)) {	
	    it++;
	    cerr << "select" << endl;
	    while (it != words.end() && !contains(*it, commands, noCommands)) {


	    }
	  }
	  //for statements of the form INSERT INTO ... VALUES (math or text, math or text, ...)
	  //getExpressions(math or text)
	  if (it != words.end() && contains(*it, insertinto)) {
	    it++;
	    cerr << "insert" << endl;
	    while (it != words.end() && !contains(*it, commands, noCommands)) {
	      if (it != words.end() && contains(*it, values)) {
			it++;
			assert_s(it->compare("(") == 0, "VALUES is not followed by (");
			it++;
			string res = "";
			//consolidates comma-separated links
			while ((it != words.end()) && !isKeyword(*it)) {
			  while ((it != words.end()) && !contains(*it, comma)) {
			    cerr << *it << endl;
				if (isOnly(*it, math, noMath)) {
					res += *it;
					oldit = it;
					it++;

					words.erase(oldit);
				} else {
					break;
				}
			  }
			  Equation eq;
			  eq.set(res);
			  res = eq.rpn();
			  if (res.compare("") != 0) {
			    words.insert(it, res);
			  }
			  if (it != words.end()) {
			    it++;
			  }
			}
			if (it != words.end()) {
				it++;
			}
			continue;
	      }
	      it++;
	    }
	  }
	  //for statements of the form UPDATE ... SET ... = math
	  //WHERE clause as with SELECT
	  //SET ignore for now; maybe do this later
	  if (it != words.end() && contains(*it, update)) {
	    cerr << "update" << endl;
	  }
	  //if (it != words.end()) {
	  //  it++;
	  //}
	  
	}

	}*/

void consolidate(list<string> & words) {

	consolidateComparisons(words);
	//cerr << "after comparisons we have " << toString(words) << "\n";
	//consolidateMath(words);
	//err << "after math we have " << toString(words) << "\n";
}

//query to parse
//delimsStay, delimsGo, keepIntact for parsing
//tables
//returns sql tokens and places in tables the tables to which this query refers
// if tables have aliases, each alias is replaced with the real name
list<string> getSQLWords(const char * query)
{
	//struct timeval starttime, endtime;


	//	gettimeofday(&starttime, NULL);

	list<string> words = parse(query, delimsStay,delimsGo, keepIntact);

	//	gettimeofday(&endtime, NULL);

//		cerr << "  parse " << timeInMSec(starttime, endtime) << "\n";

	//	gettimeofday(&starttime, NULL);
//
	//keywordsToLowerCase(words);

//	gettimeofday(&endtime, NULL);


//	cerr << "  lower case " << timeInMSec(starttime, endtime) << "\n";
//
//	gettimeofday(&starttime, NULL);

	consolidate(words);

//	gettimeofday(&endtime, NULL);


	//cerr << "  consolidate " << timeInMSec(starttime, endtime) << "\n";


	return words;

}


command getCommand(const char * queryI) throw (SyntaxError) {
	char * query = copy(queryI);

	char * commandString = strtok((char *)query, " ,;()");

	string command = toLowerCase(commandString);


	if (command.compare("create") == 0) {
		return CREATE;
	}

	if (command.compare("update") == 0) {
		return UPDATE;
	}

	if (command.compare("insert") == 0) {
		return INSERT;
	}

	if (command.compare("select") == 0) {
		return SELECT;
	}

	if (command.compare("drop") == 0) {
		return DROP;
	}

	if (command.compare("delete") == 0) {
		return DELETE;
	}

	if (command.compare("commit") == 0) {
		return COMMIT;
	}

	if (command.compare("begin") == 0) {
		return BEGIN;
	}

	if (command.compare("alter") == 0) {
		return ALTER;
	}

	return OTHER;



}


string getAlias(list<string>::iterator & it, list<string> & words) {

	if (it == words.end()) {
		return "";
	}

	if (equalsIgnoreCase(*it, "as" )) {
		it++;
		return *it;
	}

	if ((it->compare(",") == 0) || (isQuerySeparator(*it))) {
		return "";
	}

	return *it;
}

void append(list<string> & lst1, const list<string> & lst2) {
	for (list<string>::const_iterator it = lst2.begin(); it != lst2.end(); it++) {
		lst1.push_back(*it);
	}
}

list<string> concatenate(const list<string> & lst1, const list<string>  & lst2) {
	list<string> res = list<string>();

	append(res, lst1);
	append(res, lst2);

	return res;

}

string processAlias(list<string>::iterator & it, list<string> & words) {
	string res = "";

	const string terms[] = {",",")"};
	unsigned int noTerms = 2;

	if (it == words.end()) {
		return res;
	}
	if (equalsIgnoreCase(*it, "as")) {
		res = res + " as ";
		it++;
		assert_s(it != words.end(), "there should be field after as");
		res = res + *it + " ";
		it++;

		if (it == words.end()) {
			return res;
		}

		if (contains(*it, terms, noTerms)) {
			while ((it!=words.end()) && contains(*it, terms, noTerms)) {
				res = res + *it;
				it++;
			}
			return res;
		}
		if (isQuerySeparator(*it)) {
				return res;
		}
		assert_s(false, "incorrect syntax after as expression ");
		return res;
	}
	if (contains(*it, terms, noTerms)) {
		while ((it!=words.end()) && contains(*it, terms, noTerms)) {
			res = res + *it;
			it++;
		}
		return res;
	}
	if (isQuerySeparator(*it)) {
		return res;
	}

    //you have an alias without as
	res = res + " " + *it + " ";
	it++;

	if (it == words.end()) {
		return res;
	}
	if (contains(*it, terms, noTerms)) {
		while ((it!=words.end()) && contains(*it, terms, noTerms)) {
			res = res + *it;
			it++;
		}
		return res;
	}
	if (isQuerySeparator(*it)) {
		return res;
	}

	assert_s(false, "syntax error around alias or comma");

	return res;

}

string processParen(list<string>::iterator & it, list<string> & words) {
	if (!it->compare("(") == 0) {
		return "";
	}
	// field token is "("
	int nesting = 1;
	it++;
	string res = "(";
	while ((nesting > 0) && (it!=words.end())) {
			if (it->compare("(") == 0) {
				nesting++;
			}
			if (it->compare(")") == 0) {
				nesting--;
			}
			res = res + " " + *it;
			it++;
	}
	assert_s(nesting == 0, "tokens ended before parenthesis were matched \n");
	return res + " ";
}

string mirrorUntilTerm(list<string>::iterator & it, list<string> & words, string terms[], unsigned int noTerms, bool stopAfterTerm, bool skipParenBlock) {
	string res = " ";
	while ((it!=words.end()) && (!contains(*it, terms, noTerms)) ) {
		if (skipParenBlock) {
			string paren = processParen(it, words);
			if (paren.length() > 0) {
				res = res + paren;
				continue;
			}
		}
		res = res + *it + " ";
		it++;
	}

	if (it!=words.end()) {
		if (stopAfterTerm) {
			res = res + *it + " ";
			it++;
		}
	}

	return res;
}

string mirrorUntilTerm(list<string>::iterator & it, list<string> & words, const std::set<string> & terms, bool stopAfterTerm, bool skipParenBlock) {
	string res = " ";
	while ((it!=words.end()) && (terms.find(toLowerCase(*it)) == terms.end()))  {
		if (skipParenBlock) {
			string paren = processParen(it, words);
			if (paren.length() > 0) {
				res = res + paren;
				continue;
			}
		}
		res = res + *it + " ";
		it++;
	}

	if (it!=words.end()) {
		if (stopAfterTerm) {
			res = res + *it + " ";
			it++;
		}
	}

	return res;
}

list<string>::iterator itAtKeyword(list<string> & lst, string keyword) {
	list<string>::iterator it = lst.begin();
	while (it != lst.end()) {
		if (equalsIgnoreCase(*it, keyword)) {
			return it;
		} else {
			it++;
		}
	}
	return it;
}

string getBeforeChar(string str, char c) {
	unsigned int pos = str.find(c);
	if (pos != string::npos) {
		return str.substr(0, pos);
	} else {
		return "";
	}
}

bool isQuerySeparator(string st) {
	return (parserMeta.querySeparators_p.find(toLowerCase(st)) != parserMeta.querySeparators_p.end());
}

bool isAgg(string value) {
	return contains(value, aggregates, noAggregates);
}

bool contains(string token, const string * values, unsigned int noValues) {
	for (unsigned int i = 0; i < noValues; i++) {
		if (equalsIgnoreCase(token, values[i])) {
			return true;
		}
	}
	return false;
}
bool contains(string token, list<string> & values) {
	for (list<string>::iterator it = values.begin(); it != values.end(); it++) {
		if (equalsIgnoreCase(*it, token)){
			return true;
		}
	}

	return false;
}
bool isOnly(string token, const string * values, unsigned int noValues) {
        const char * str = getCStr(token);
	for (unsigned int j = 0; j < token.size(); j++) {
	  bool is_value = false;
	  for (unsigned int i = 0; i < noValues; i++) {
	        string test = "";
		test += str[j];
	        if (equalsIgnoreCase(test, values[i])) {
		  is_value = true;
		}
	  }
	  if (!is_value) {
	    cerr << str[j] << endl;
	    return false;
	  }
	}
	return true;
}

bool isKeyword(string token) {
	return (parserMeta.clauseKeywords_p.find(toLowerCase(token)) != parserMeta.clauseKeywords_p.end());
}

const char * removeString(const char * input, string toremove) {
	string data(input);
	string newdata = "";
	unsigned int pos;
	while ((pos = data.find(toremove)) != string::npos) {
		newdata = data.substr(0, pos);
		if (pos == data.length() - 1) {
			data = "";
			break;
		}
		data = data.substr(pos+toremove.length(), data.length() - pos - toremove.length());
	}
	newdata = newdata +  data;
	return getCStr(newdata);
}


bool contains(string token1, string token2, list<pair<string, string> > & lst) {
	for (list<pair<string, string> >::iterator it = lst.begin(); it != lst.end(); it++) {
		if ((it->first.compare(token1) == 0) && (it->second.compare(token2) == 0)){
			return true;
		}
	}

	return false;


}

void addIfNotContained(string token, list<string> & lst) {
	if (!contains(token, lst)) {
		lst.push_back(token);
	}
}
void addIfNotContained(string token1, string token2, list<pair<string, string> > & lst) {

	if (!contains(token1, token2, lst)) {
		lst.push_back(pair<string, string>(token1, token2));
	}
}

string removeApostrophe(string data) {
	if (data[0] == '\'') {
		assert_s(data[data.length()-1] == '\'', "not matching ' ' \n");
		return data.substr(1, data.length() - 2);
	} else {
		return data;
	}
}

bool hasApostrophe(string data) {
	return ((data[0] == '\'') && (data[data.length()-1] == '\''));
}

unsigned char * homomorphicAdd (unsigned char * val1,  unsigned char * val2, unsigned char * valn2, int len) {
	ZZ z1 = ZZFromBytes(val1, len);
	ZZ z2 = ZZFromBytes(val2, len);
	ZZ n2 = ZZFromBytes(valn2, len);
	ZZ res = MulMod(z1, z2, n2);
	return BytesFromZZ(res, len);
}

string toLowerCase(string token) {
	string res = "";
	for (unsigned int i = 0; i < token.length(); i++) {
		char c;
		if (isalpha(token[i])) {
		    c = tolower(token[i]);
		} else {
			c = token[i];
		}
		res = res + c;
	}
	return res;
}

bool equalsIgnoreCase(string s1, string s2) {
	return (toLowerCase(s1).compare(toLowerCase(s2)) == 0);
}

void keywordsToLowerCase(list<string> & lst) {
	list<string>::iterator it = lst.begin();

	while (it != lst.end()) {
		if ((!hasApostrophe(*it)) && isKeyword(*it)) {
				*it = toLowerCase(*it);
		}
		it++;
	}
}


/***************************
*           Operations
****************************/

bool Operation::isDET(string op) {

	string dets[] = {"=", "<>", "in", "!="};
	unsigned int noDets = 4;

	return contains(op, dets, noDets);
}

bool Operation::isIN(string op) {

	if (equalsIgnoreCase(op,"in")) {
		return true;
	}
	return false;

}

bool Operation::isOPE(string op) {

	string opes[] = {"<", ">", "<=", ">="};
	unsigned int noOpes = 4;

	return contains(op, opes, noOpes);
}

bool Operation::isILIKE(string op) {
	string ilikes[] = {"ilike", "like"};

	return contains(op, ilikes, 2);
}

bool Operation::isOp(string op) {
	return (isDET(op) || isOPE(op) || isILIKE(op));
}



/*
bool addAgg(string currentField, string & aggregateFields) {
	 currentField = "." + currentField + " ";
     if (aggregateFields.length() == 0) {
    	 return true;
     }

     if (aggregateFields.find(currentField) != string::npos) {
    	 return true;
     }

     return false;
}
*/