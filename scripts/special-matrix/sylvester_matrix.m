## Copyright (C) 1996, 1997, 1998, 1999, 2000, 2002, 2004, 2005, 2006,
##               2007 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {} sylvester_matrix (@var{k})
## Return the Sylvester matrix of order
## @iftex
## @tex
## $n = 2^k$.
## @end tex
## @end iftex
## @ifinfo
## n = 2^k.
## @end ifinfo
## @seealso{hankel, vander, hilb, invhilb, toeplitz}
## @end deftypefn

## Author: jwe

function retval = sylvester_matrix (k)

  if (nargin != 1)
    print_usage ();
  endif

  if (isscalar (k))
    if (k < 1)
      retval = 1;
    else
      tmp = sylvester_matrix (k-1);
      retval = [tmp, tmp; tmp, -tmp];
    endif
  else
    error ("sylvester_matrix: expecting scalar argument");
  endif

endfunction

%!assert((sylvester_matrix (1) == [1, 1; 1, -1]
%! && (sylvester_matrix (2)
%! == [1, 1, 1, 1; 1, -1, 1, -1; 1, 1, -1, -1; 1, -1, -1, 1])));

%!error sylvester_matrix ([1, 2; 3, 4]);

%!error sylvester_matrix ();

%!error sylvester_matrix (1, 2);

