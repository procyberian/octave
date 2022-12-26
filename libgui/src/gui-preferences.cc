////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017-2022 The Octave Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution or <https://octave.org/copyright/>.
//
// This file is part of Octave.
//
// Octave is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Octave is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include "gui-preferences-sc.h"
#include "gui-preferences.h"

sc_pref::sc_pref (const QString& description, const QString& settings_key,
                  Qt::Key def)
  : m_description (description), m_settings_key (settings_key), m_def (def),
    m_def_std (QKeySequence::UnknownKey)
{
  all_shortcut_preferences::insert (settings_key, *this);
}

sc_pref::sc_pref (const QString& description, const QString& settings_key,
                  unsigned int def)
  : m_description (description), m_settings_key (settings_key), m_def (def),
    m_def_std (QKeySequence::UnknownKey)
{
  all_shortcut_preferences::insert (settings_key, *this);
}

sc_pref::sc_pref (const QString& description, const QString& settings_key,
                  QKeySequence::StandardKey def_std)
  : m_description (description), m_settings_key (settings_key), m_def (0),
    m_def_std (def_std)
{
  all_shortcut_preferences::insert (settings_key, *this);
}

all_shortcut_preferences *all_shortcut_preferences::s_instance = nullptr;

void all_shortcut_preferences::insert (const QString& settings_key,
                                       const sc_pref& scpref)
{
  ensure_instance ();

  s_instance->do_insert (settings_key, scpref);
}

void all_shortcut_preferences::do_insert (const QString& settings_key,
                                          const sc_pref& scpref)
{
  m_hash.insert (settings_key, scpref);
}

void all_shortcut_preferences::ensure_instance (void)
{
  if (! s_instance)
    s_instance = new all_shortcut_preferences ();
}
