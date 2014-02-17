/* @@@LICENSE
*
*      Copyright (c) 2009-2013 LG Electronics, Inc.
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


#ifndef MOJDBQUERYFILTER_H_
#define MOJDBQUERYFILTER_H_

#include "db/MojDbDefs.h"
#include "db/MojDbQuery.h"

class MojDbQueryFilter
{
public:
	MojDbQueryFilter();
	~MojDbQueryFilter();

	MojErr init(const MojDbQuery& query);
    MojErr test(const MojObject& obj, bool & ret) const;
    MojErr findValue(const MojObject obj, const MojString* begin, const MojString* end, MojObject& valOut, bool & ret) const;

private:
    class FindSubStringResult
    {
    public:
        FindSubStringResult()               : m_pos(-1), m_srcIsEmpty(false), m_subStringIsEmpty(false) {}
        explicit FindSubStringResult(int p) : m_pos( p), m_srcIsEmpty(false), m_subStringIsEmpty(false) {}
        explicit FindSubStringResult(bool src, bool sub) : m_pos( -1), m_srcIsEmpty(src), m_subStringIsEmpty(sub) {}

        bool isFound() const
        {
            return m_pos > -1 || ( m_srcIsEmpty  && m_subStringIsEmpty );
        }

        int pos() const
        {
            return m_pos;
        }

        bool srcIsEmpty() const {return m_srcIsEmpty;}
        bool subStringIsEmpty() const {return m_subStringIsEmpty;}
    private:
        int m_pos;
        bool m_srcIsEmpty;
        bool m_subStringIsEmpty;
    };

    static MojErr testLower(const MojDbQuery::WhereClause& clause, const MojObject& val, bool & ret);
    static MojErr testUpper(const MojDbQuery::WhereClause& clause, const MojObject& val, bool & ret);
    static MojErr findSubString(const MojObject& src, const MojObject& subString, FindSubStringResult& ret);

	MojDbQuery::WhereMap m_clauses;
};

#endif /* MOJDBQUERYFILTER_H_ */
