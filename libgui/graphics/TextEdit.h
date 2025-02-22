////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2011-2024 The Octave Project Developers
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

#if ! defined (octave_TextEdit_h)
#define octave_TextEdit_h 1

#include <QTextEdit>

OCTAVE_BEGIN_NAMESPACE(octave)

class TextEdit : public QTextEdit
{
  Q_OBJECT

public:
  TextEdit (QWidget *xparent) : QTextEdit (xparent) { }
  ~TextEdit () = default;

Q_SIGNALS:
  void editingFinished ();
  void returnPressed ();

protected:
  void focusOutEvent (QFocusEvent *event);
  void keyPressEvent (QKeyEvent *event);
};

OCTAVE_END_NAMESPACE(octave)

#endif
