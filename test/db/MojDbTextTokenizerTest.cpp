/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */


#include "MojDbTextTokenizerTest.h"
#include "db/MojDbKey.h"
#include "db/MojDbTextTokenizer.h"

MojDbTextTokenizerTest::MojDbTextTokenizerTest()
: MojTestCase(_T("MojDbTextTokenizer"))
{
}

MojErr MojDbTextTokenizerTest::run()
{
	MojErr err = englishTest();
	MojErrCheck(err);

	return MojErrNone;
}

MojErr MojDbTextTokenizerTest::englishTest()
{
	MojErr err = check(_T(""), _T("[]"));
	MojTestErrCheck(err);
	err = check(_T(" "), _T("[]"));
	MojTestErrCheck(err);
	err = check(_T(" \t\r\n\f\v()*&^%$#@!~`?/.,,._-+=[]{}\\|"), _T("[]"));
	MojTestErrCheck(err);
	err = check(_T("a"), _T("[\"a\"]"));
	MojTestErrCheck(err);
	err = check(_T("hello"), _T("[\"hello\"]"));
	MojTestErrCheck(err);
	err = check(_T("hello world"), _T("[\"hello\",\"world\"]"));
	MojTestErrCheck(err);
	err = check(_T(" hello world "), _T("[\"hello\",\"world\"]"));
	MojTestErrCheck(err);
	err = check(_T(".-.-hello^^&^&world{}{}{}"), _T("[\"hello\",\"world\"]"));
	MojTestErrCheck(err);
	err = check(_T("the quick brown fox jumped over the lazy yellow dog."),
			_T("[\"the\",\"quick\",\"brown\",\"fox\",\"jumped\",\"over\",\"lazy\",\"yellow\",\"dog\"]"));
	MojTestErrCheck(err);

	return MojErrNone;
}

MojErr MojDbTextTokenizerTest::check(const MojChar* text, const MojChar* tokens)
{
	// tokenize string
	MojString textStr;
	MojErr err = textStr.assign(text);
	MojTestErrCheck(err);
	MojSet<MojDbKey> set;
	MojRefCountedPtr<MojDbTextTokenizer> tokenizer(new MojDbTextTokenizer);
	MojAllocCheck(tokenizer.get());
	err = tokenizer->init(_T("en_US"));
	MojTestErrCheck(err);
	err = tokenizer->tokenize(textStr, NULL, set);
	MojTestErrCheck(err);
	// check that tokens match
	MojObject obj;
	err = obj.fromJson(tokens);
	MojTestErrCheck(err);
	MojSize objSize = obj.size();
	MojSize setSize = set.size();
	MojTestAssert(objSize == setSize);
	for (MojObject::ConstArrayIterator i = obj.arrayBegin(); i != obj.arrayEnd(); ++i) {
		MojDbKey key;
		err = key.assign(*i);
		MojTestErrCheck(err);
		MojTestAssert(set.contains(key));
	}
	return MojErrNone;
}
