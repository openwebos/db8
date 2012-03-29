/* @@@LICENSE
*
*      Copyright (c) 2012 Hewlett-Packard Development Company, L.P.
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


#ifndef MOJNOCOPY_H_
#define MOJNOCOPY_H_

class MojNoCopy
{
protected:
	MojNoCopy() {}

private:
	MojNoCopy(const MojNoCopy&);
	MojNoCopy& operator=(const MojNoCopy&);
};

#endif /* MOJNOCOPY_H_ */
