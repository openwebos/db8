/* @@@LICENSE
*
*      Copyright (c) 2013 LG Electronics, Inc.
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

#include "db/MojDbKindIdList.h"
#include <boost/tokenizer.hpp>

std::list<MojString>::iterator MojDbKindIdList::begin (void)
{
    return listKindIds.begin();
}

std::list<MojString>::iterator MojDbKindIdList::end (void)
{
    return listKindIds.end();
}

bool MojDbKindIdList::isExist (const MojString& kindId)
{
    std::list<MojString>::iterator it;
    it = std::find(listKindIds.begin(), listKindIds.end(), kindId);
    return (it != listKindIds.end());
}

void MojDbKindIdList::add (const MojString& kindId)
{
    listKindIds.push_back(kindId);
}

void MojDbKindIdList::remove (const MojString& kindId)
{
    std::list<MojString>::iterator it;
    it = std::find(listKindIds.begin(), listKindIds.end(), kindId);

    if(it != listKindIds.end())
    {
        listKindIds.erase(it);
    }
}

MojErr MojDbKindIdList::toString (MojString& str) const
{
    std::list<MojString>::const_iterator it;

    for (it = listKindIds.begin(); it != listKindIds.end(); ++it)
    {
        if(it != listKindIds.begin())
        {
            MojErrCheck(str.append(","));
        }

        MojErrCheck(str.append(*it));
    }

    return MojErrNone;
}

MojErr MojDbKindIdList::fromString (const MojString& str)
{
    listKindIds.clear();

    if(str.empty())
    {
        return MojErrNone;
    }

    MojString item;
    std::string testStr = str.data();
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char> > tokens(testStr, sep);
    boost::tokenizer<boost::char_separator<char> >::iterator it;

    for(it = tokens.begin(); it != tokens.end(); ++it)
    {
        MojErrCheck(item.assign((*it).c_str()));
        listKindIds.push_back(item);
    }

    return MojErrNone;
}
