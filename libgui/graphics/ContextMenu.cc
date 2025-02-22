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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <QMenu>

#include "ContextMenu.h"
#include "QtHandlesUtils.h"
#include "qt-graphics-toolkit.h"

#include "interpreter.h"

OCTAVE_BEGIN_NAMESPACE(octave)

ContextMenu *
ContextMenu::create (octave::interpreter& interp, const graphics_object& go)
{
  Object *xparent = parentObject (interp, go);

  if (xparent)
    {
      QWidget *w = xparent->qWidget<QWidget> ();

      return new ContextMenu (interp, go, new QMenu (w));
    }

  return nullptr;
}

ContextMenu::ContextMenu (octave::interpreter& interp,
                          const graphics_object& go, QMenu *xmenu)
  : Object (interp, go, xmenu)
{
  xmenu->setAutoFillBackground (true);

  connect (xmenu, &QMenu::aboutToShow, this, &ContextMenu::aboutToShow);
  connect (xmenu, &QMenu::aboutToHide, this, &ContextMenu::aboutToHide);
}

ContextMenu::~ContextMenu ()
{ }

void
ContextMenu::update (int pId)
{
  uicontextmenu::properties& up = properties<uicontextmenu> ();
  QMenu *xmenu = qWidget<QMenu> ();

  switch (pId)
    {
    case base_properties::ID_VISIBLE:
      if (up.is_visible ())
        {
          Matrix pos = up.get_position ().matrix_value ();
          QWidget *parentW = xmenu->parentWidget ();
          QPoint pt;

          pt.rx () = octave::math::round (pos(0));
          pt.ry () = parentW->height () - octave::math::round (pos(1));
          pt = parentW->mapToGlobal (pt);

          xmenu->popup (pt);
        }
      else
        xmenu->hide ();
      break;
    default:
      Object::update (pId);
      break;
    }
}

void
ContextMenu::aboutToShow ()
{
  Q_EMIT gh_callback_event (m_handle, "callback");
  Q_EMIT gh_set_event (m_handle, "visible", "on", false);
}

void
ContextMenu::aboutToHide ()
{
  Q_EMIT gh_set_event (m_handle, "visible", "off", false);
}

QWidget *
ContextMenu::menu ()
{
  return qWidget<QWidget> ();
}

void
ContextMenu::executeAt (octave::interpreter& interp,
                        const base_properties& props, const QPoint& pt)
{
  graphics_handle h = props.get_uicontextmenu ();

  if (h.ok ())
    {
      gh_manager& gh_mgr = interp.get_gh_manager ();
      octave::autolock guard (gh_mgr.graphics_lock ());

      graphics_object go = gh_mgr.get_object (h);

      if (go.valid_object ())
        {
          ContextMenu *cMenu =
            dynamic_cast<ContextMenu *> (qt_graphics_toolkit::toolkitObject (go));

          if (cMenu)
            {
              QMenu *menu = cMenu->qWidget<QMenu> ();

              if (menu)
                menu->popup (pt);
            }
        }
    }
}

OCTAVE_END_NAMESPACE(octave)
