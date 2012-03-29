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


#ifndef MOJAUTOPTRINTERNAL_H_
#define MOJAUTOPTRINTERNAL_H_

template<class T, class DTOR>
void MojSharedPtrBase<T, DTOR>::destroy()
{
	if (m_impl && --(m_impl->m_refcount) == 0)
		delete m_impl;
}

template<class T, class DTOR>
MojErr MojSharedPtrBase<T, DTOR>::reset(T* p)
{
	if (m_impl && --(m_impl->m_refcount) == 0) {
		if (p) {
			m_impl->destroy();
			m_impl->m_p = p;
			m_impl->m_refcount = 1;
		} else {
			delete m_impl;
		}
	} else if (p) {
		m_impl = new MojSharedPtrImpl<T, DTOR>(p);
		if (!m_impl) {
			DTOR()(p);
			MojErrThrow(MojErrNoMem);
		}
	} else {
		m_impl = NULL;
	}

	return MojErrNone;
}

template<class T, class DTOR>
MojErr MojSharedPtrBase<T, DTOR>::resetChecked(T* p)
{
	if (p == NULL)
		MojErrThrow(MojErrNoMem);
	MojErr err = reset(p);
	MojErrCheck(err);

	return MojErrNone;
}

template<class T, class DTOR>
void MojSharedPtrBase<T, DTOR>::assign(MojSharedPtrImpl<T, DTOR>* impl)
{
	destroy();
	m_impl = impl;
	init();
}

#endif /* MOJAUTOPTRINTERNAL_H_ */
