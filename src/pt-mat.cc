/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include "defun.h"
#include "error.h"
#include "oct-obj.h"
#include "pt-arg-list.h"
#include "pt-bp.h"
#include "pt-exp.h"
#include "pt-mat.h"
#include "pt-walk.h"
#include "utils.h"
#include "ov.h"
#include "variables.h"

// If TRUE, print a warning message for empty elements in a matrix list.
static bool Vwarn_empty_list_elements;

// The character to fill with when creating string arrays.
char Vstring_fill_char = ' ';

// General matrices.  This list type is much more work to handle than
// constant matrices, but it allows us to construct matrices from
// other matrices, variables, and functions.

// But first, some internal classes that make our job much easier.

class
tm_row_const
{
private:

  class
  tm_row_const_rep : public octave_base_list<octave_value>
  {
  public:

    tm_row_const_rep (void)
      : count (1), nr (0), nc (0),
	all_str (false), some_str (false), is_cmplx (false),
	all_mt (true), ok (false) { }

    tm_row_const_rep (const tree_argument_list& row)
      : count (1), nr (0), nc (0),
	all_str (false), some_str (false), is_cmplx (false),
	all_mt (true), ok (false)
    { init (row); }

    ~tm_row_const_rep (void) { }

    int count;

    int nr;
    int nc;

    bool all_str;
    bool some_str;
    bool is_cmplx;
    bool all_mt;

    bool ok;

    void init (const tree_argument_list&);

  private:

    tm_row_const_rep (const tm_row_const_rep&);

    tm_row_const_rep& operator = (const tm_row_const_rep&);

    void eval_error (const char *msg, int l, int c,
		     int x = -1, int y = -1) const;

    void eval_warning (const char *msg, int l, int c) const;
  };

public:

  typedef tm_row_const_rep::iterator iterator;
  typedef tm_row_const_rep::const_iterator const_iterator;

  tm_row_const (void)
    : rep (0) { }

  tm_row_const (const tree_argument_list& row)
    : rep (new tm_row_const_rep (row)) { }

  tm_row_const (const tm_row_const& x)
    : rep (x.rep)
  {
    if (rep)
      rep->count++;
  }

  tm_row_const& operator = (const tm_row_const& x)
  {
    if (this != &x && rep != x.rep)
      {
	if (rep && --rep->count == 0)
	  delete rep;

	rep = x.rep;

	if (rep)
	  rep->count++;
      }

    return *this;
  }

  ~tm_row_const (void)
  {
    if (rep && --rep->count == 0)
      delete rep;
  }

  int rows (void) { return rep->nr; }
  int cols (void) { return rep->nc; }

  bool all_strings_p (void) const { return rep->all_str; }
  bool some_strings_p (void) const { return rep->some_str; }
  bool complex_p (void) const { return rep->is_cmplx; }
  bool all_empty_p (void) const { return rep->all_mt; }

  operator bool () const { return (rep && rep->ok); }

  iterator begin (void) { return rep->begin (); }
  const_iterator begin (void) const { return rep->begin (); }

  iterator end (void) { return rep->end (); }
  const_iterator end (void) const { return rep->end (); }

private:

  tm_row_const_rep *rep;
};

void
tm_row_const::tm_row_const_rep::init (const tree_argument_list& row)
{
  all_str = true;

  bool first_elem = true;

  for (tree_argument_list::const_iterator p = row.begin ();
       p != row.end ();
       p++)
    {
      tree_expression *elt = *p;

      octave_value tmp = elt->rvalue ();

      if (error_state || tmp.is_undefined ())
	break;
      else
	{
	  int this_elt_nr = tmp.rows ();
	  int this_elt_nc = tmp.columns ();

	  if (this_elt_nr > 0 || this_elt_nc > 0)
	    {
	      all_mt = false;

	      if (first_elem)
		{
		  first_elem = false;

		  nr = this_elt_nr;
		}
	      else if (this_elt_nr != nr)
		{
		  eval_error ("number of rows must match",
			      elt->line (), elt->column (), this_elt_nr, nr);
		  break;
		}

	      nc += this_elt_nc;

	      append (tmp);
	    }
	  else if (Vwarn_empty_list_elements)
	    eval_warning ("empty matrix found in matrix list",
			  elt->line (), elt->column ());

	  if (all_str && ! tmp.is_string ())
	    all_str = false;

	  if (! some_str && tmp.is_string ())
	    some_str = true;

	  if (! is_cmplx && tmp.is_complex_type ())
	    is_cmplx = true;
	}
    }

  ok = ! error_state;
}

void
tm_row_const::tm_row_const_rep::eval_error (const char *msg, int l,
					    int c, int x, int y) const
{
  if (l == -1 && c == -1)
    {
      if (x == -1 || y == -1)
	::error ("%s", msg);
      else
	::error ("%s (%d != %d)", msg, x, y);
    }
  else
    {
      if (x == -1 || y == -1)
	::error ("%s near line %d, column %d", msg, l, c);
      else
	::error ("%s (%d != %d) near line %d, column %d", msg, x, y, l, c);
    }
}

void
tm_row_const::tm_row_const_rep::eval_warning (const char *msg, int l,
					      int c) const
{
  if (l == -1 && c == -1)
    ::warning ("%s", msg);
  else
    ::warning ("%s near line %d, column %d", msg, l, c);
}

class
tm_const : public octave_base_list<tm_row_const>
{
public:

  tm_const (const tree_matrix& tm)
    : nr (0), nc (0),
      all_str (false), some_str (false), is_cmplx (false),
      all_mt (true), ok (false)
      { init (tm); }

  ~tm_const (void) { }

  int rows (void) const { return nr; }
  int cols (void) const { return nc; }

  bool all_strings_p (void) const { return all_str; }
  bool some_strings_p (void) const { return some_str; }
  bool complex_p (void) const { return is_cmplx; }
  bool all_empty_p (void) const { return all_mt; }

  operator bool () const { return ok; }

private:

  int nr;
  int nc;

  bool all_str;
  bool some_str;
  bool is_cmplx;
  bool all_mt;

  bool ok;

  tm_const (void);

  tm_const (const tm_const&);

  tm_const& operator = (const tm_const&);

  void init (const tree_matrix& tm);
};

void
tm_const::init (const tree_matrix& tm)
{
  all_str = true;

  bool first_elem = true;

  // Just eval and figure out if what we have is complex or all
  // strings.  We can't check columns until we know that this is a
  // numeric matrix -- collections of strings can have elements of
  // different lengths.

  for (tree_matrix::const_iterator p = tm.begin (); p != tm.end (); p++)
    {
      tree_argument_list *elt = *p;

      tm_row_const tmp (*elt);

      if (tmp)
	{
	  if (all_str && ! tmp.all_strings_p ())
	    all_str = false;

	  if (! some_str && tmp.some_strings_p ())
	    some_str = true;

	  if (! is_cmplx && tmp.complex_p ())
	    is_cmplx = true;

	  if (all_mt && ! tmp.all_empty_p ())
	    all_mt = false;

	  append (tmp);
	}
      else
	break;
    }

  if (! error_state)
    {
      for (iterator p = begin (); p != end (); p++)
	{
	  tm_row_const elt = *p;

	  int this_elt_nr = elt.rows ();
	  int this_elt_nc = elt.cols ();

	  if (this_elt_nr > 0 || this_elt_nc > 0)
	    {
	      all_mt = false;

	      if (first_elem)
		{
		  first_elem = false;

		  nc = this_elt_nc;
		}
	      else if (all_str)
		{
		  if (this_elt_nc > nc)
		    nc = this_elt_nc;
		}
	      else if (this_elt_nc != nc)
		{
		  ::error ("number of columns must match (%d != %d)",
			   this_elt_nc, nc);
		  break;
		}

	      nr += this_elt_nr;
	    }
	  else if (Vwarn_empty_list_elements)
	    warning ("empty matrix found in matrix list");
	}
    }

  ok = ! error_state;
}

tree_matrix::~tree_matrix (void)
{
  while (! empty ())
    {
      iterator p = begin ();
      delete *p;
      erase (p);
    }
}

bool
tree_matrix::has_magic_end (void) const
{
  for (const_iterator p = begin (); p != end (); p++)
    {
      tree_argument_list *elt = *p;

      if (elt && elt->has_magic_end ())
	return true;
    }

  return false;
}

bool
tree_matrix::all_elements_are_constant (void) const
{
  for (const_iterator p = begin (); p != end (); p++)
    {
      tree_argument_list *elt = *p;

      if (! elt->all_elements_are_constant ())
	return false;
    }

  return true;
}

// Just about as ugly as it gets.
// Less ugly than before, anyway.
// Looking better all the time.

octave_value_list
tree_matrix::rvalue (int nargout)
{
  octave_value_list retval;

  MAYBE_DO_BREAKPOINT;

  if (nargout > 1)
    error ("invalid number of output arguments for matrix list");
  else
    retval = rvalue ();

  return retval;
}

octave_value
tree_matrix::rvalue (void)
{
  octave_value retval;

  tm_const tmp (*this);

  bool all_strings_p = false;
  bool some_strings_p = false;
  bool all_empty_p = false;

  bool frc_str_conv = false;

  if (tmp)
    {
      int nr = tmp.rows ();
      int nc = tmp.cols ();

      Matrix m;
      ComplexMatrix cm;
      charMatrix chm;

      // Now, extract the values from the individual elements and
      // insert them in the result matrix.

      bool found_complex = tmp.complex_p ();

      all_strings_p = tmp.all_strings_p ();
      some_strings_p = tmp.some_strings_p ();
      all_empty_p = tmp.all_empty_p ();

      frc_str_conv = some_strings_p;

      if (all_strings_p)
	chm.resize (nr, nc, Vstring_fill_char);
      else if (found_complex)
	cm.resize (nr, nc, 0.0);
      else
	m.resize (nr, nc, 0.0);

      int put_row = 0;

      for (tm_const::iterator p = tmp.begin (); p != tmp.end (); p++)
	{
	  int put_col = 0;

	  tm_row_const row = *p;

	  for (tm_row_const::iterator q = row.begin (); q != row.end (); q++)
	    {
	      octave_value elt = *q;

	      if (found_complex)
		{
		  if (elt.is_real_scalar ())
		    cm (put_row, put_col) = elt.double_value ();
		  else if (elt.is_real_matrix () || elt.is_range ())
		    cm.insert (elt.matrix_value (), put_row, put_col);
		  else if (elt.is_complex_scalar ())
		    cm (put_row, put_col) = elt.complex_value ();
		  else
		    {
		      ComplexMatrix cm_elt = elt.complex_matrix_value ();

		      if (error_state)
			goto done;

		      cm.insert (cm_elt, put_row, put_col);
		    }
		}
	      else
		{
		  if (elt.is_real_scalar ())
		    m (put_row, put_col) = elt.double_value ();
		  else if (elt.is_string () && all_strings_p)
		    {
		      charMatrix chm_elt = elt.char_matrix_value ();

		      if (error_state)
			goto done;

		      chm.insert (chm_elt, put_row, put_col);
		    }
		  else
		    {
		      Matrix m_elt = elt.matrix_value (frc_str_conv);

		      if (error_state)
			goto done;

		      m.insert (m_elt, put_row, put_col);
		    }
		}

	      if (all_strings_p && chm.rows () > 0 && chm.cols () > 0)
		retval = octave_value (chm, true);
	      else if (found_complex)
		retval = cm;
	      else
		retval = m;

	      put_col += elt.columns ();
	    }

	  put_row += row.rows ();
	}
    }

done:

  if (! error_state)
    {
      if (retval.is_undefined () && all_empty_p)
	{
	  if (all_strings_p)
	    retval = "";
	  else
	    retval = Matrix ();
	}
      else if (frc_str_conv && ! retval.is_string ())
	retval = retval.convert_to_str ();
    }

  return retval;
}

void
tree_matrix::accept (tree_walker& tw)
{
  tw.visit_matrix (*this);
}

static int
warn_empty_list_elements (void)
{
  Vwarn_empty_list_elements = check_preference ("warn_empty_list_elements");

  return 0;
}

static int
string_fill_char (void)
{
  int status = 0;

  std::string s = builtin_string_variable ("string_fill_char");

  switch (s.length ())
    {
    case 1:
      Vstring_fill_char = s[0];
      break;

    case 0:
      Vstring_fill_char = '\0';
      break;

    default:
      warning ("string_fill_char must be a single character");
      status = -1;
      break;
    }

  return status;
}

void
symbols_of_pt_mat (void)
{
  DEFVAR (string_fill_char, " ", string_fill_char,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} string_fill_char\n\
The value of this variable is used to pad all strings in a string matrix\n\
to the same length.  It should be a single character.  The default value\n\
is @code{\" \"} (a single space).  For example,\n\
\n\
@example\n\
@group\n\
string_fill_char = \"X\";\n\
[ \"these\"; \"are\"; \"strings\" ]\n\
     @result{} \"theseXX\"\n\
        \"areXXXX\"\n\
        \"strings\"\n\
@end group\n\
@end example\n\
@end defvr");

  DEFVAR (warn_empty_list_elements, false, warn_empty_list_elements,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} warn_empty_list_elements\n\
If the value of @code{warn_empty_list_elements} is nonzero, print a\n\
warning when an empty matrix is found in a matrix list.  For example,\n\
\n\
@example\n\
a = [1, [], 3, [], 5]\n\
@end example\n\
\n\
@noindent\n\
The default value is 0.\n\
@end defvr");

}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
