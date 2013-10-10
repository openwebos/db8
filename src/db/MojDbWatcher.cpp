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


#include "db/MojDbWatcher.h"
#include "db/MojDbIndex.h"
#include "db/MojDb.h"

MojDbWatcher::MojDbWatcher(Signal::SlotRef handler)
: m_signal(this),
  m_desc(false),
  m_state(StateInvalid),
  m_index(NULL)
{
	m_signal.connect(handler);
}

MojDbWatcher::~MojDbWatcher()
{
}

void MojDbWatcher::init(MojDbIndex* index, const RangeVec& ranges, bool desc, bool active)
{
	MojAssert(index);
	MojAssert(!m_index);
	MojThreadGuard guard(m_mutex);

	m_index = index;
	m_ranges = ranges;
	m_desc = desc;
	m_state = active ? StateActive : StatePending;
}

MojErr MojDbWatcher::activate(const MojDbKey& limitKey)
{
	MojAssert(m_index);
	MojThreadGuard guard(m_mutex);

	m_state = StateActive;
	bool fired = !m_fireKey.empty();
	bool inRange = fired &&
			(limitKey.empty() ||
			(m_desc && m_fireKey >= limitKey) ||
			(!m_desc && m_fireKey <= limitKey));
	if (inRange) {
		// we were fired before activation, so if the maxKey our cursor returned
		// is >= the minKey with which we were fired, go ahead and do the fire
		MojErr err = fireImpl();
		MojErrCheck(err);
	} else {
		// keep limit so we can reject fires for larger keys
		m_limitKey = limitKey;
	}

	MojLogDebug(MojDb::s_log, _T("Watcher_activate: state= %d; fired = %d; inrange = %d; index name = %s; domain = %s\n"), (int)m_state,
				(int)fired, (int)inRange, ((m_index) ? m_index->name().data(): NULL), ((m_domain) ? m_domain.data(): NULL));
	return MojErrNone;
}

MojErr MojDbWatcher::fire(const MojDbKey& key)
{
	MojThreadGuard guard(m_mutex);

	// ignore all fires greater than our key
	bool limited = !m_limitKey.empty();
	bool inRange = !limited ||
			(m_desc && key >= m_limitKey) ||
			(!m_desc && key <= m_limitKey);

	MojLogDebug(MojDb::s_log, _T("Watcher_fire: state= %d; inrange = %d; limited = %d; index name = %s; domain = %s\n"), (int)m_state,
				(int)inRange, (int)limited, ((m_index) ? m_index->name().data() : NULL), ((m_domain) ? m_domain.data() : NULL));
	if (!inRange)
		return MojErrNone;

	if (m_state == StateActive) {
		MojErr err = fireImpl();
		MojErrCheck(err);
	} else if (m_state == StatePending) {
		// keep min fire key (or max for desc)
		bool save = (m_desc && key > m_fireKey) ||
				(!m_desc && (m_fireKey.empty() || key < m_fireKey));
		if (save) {
			m_fireKey = key;
		}
	}
	return MojErrNone;
}

MojErr MojDbWatcher::handleCancel()
{
	MojThreadGuard guard(m_mutex);

	MojLogDebug(MojDb::s_log, _T("Watcher_handleCancel: state= %d; index name = %s; domain = %s\n"), (int)m_state,
				((m_index) ? m_index->name().data() : NULL), ((m_domain) ? m_domain.data() : NULL));

	if (m_index == NULL)
		MojErrThrow(MojErrDbWatcherNotRegistered);
	if (m_state != StateInvalid) {
		MojErr err = invalidate();
		MojErrCheck(err);
	}
	return MojErrNone;
}

MojErr MojDbWatcher::fireImpl()
{
	MojAssertMutexLocked(m_mutex);

	MojErr err = invalidate();
	MojErrCheck(err);
	err = m_signal.fire();
	MojErrCatchAll(err);
	
	MojLogDebug(MojDb::s_log, _T("Watcher_fired!!: err = %d; state= %d; index name = %s; domain = %s\n"), (int)err, (int)m_state,
				((m_index) ? m_index->name().data() : NULL), ((m_domain) ? m_domain.data() : NULL));
	return MojErrNone;
}

MojErr MojDbWatcher::invalidate()
{
	MojAssertMutexLocked(m_mutex);
	MojAssert(m_index);
	MojAssert(m_state == StateActive);

	MojLogDebug(MojDb::s_log, _T("Watcher_invalidate: state= %d; index name = %s; domain = %s\n"), (int)m_state,
			((m_index) ? m_index->name().data() : NULL),((m_domain) ? m_domain.data() : NULL));
	// index always drops its mutex before firing, so
	// it's ok to hold our mutex while calling into index
	MojDbIndex* index = m_index;
	m_index = NULL;
	m_state = StateInvalid;
	MojErr err = index->cancelWatch(this);
	MojErrCheck(err);


	return MojErrNone;
}
