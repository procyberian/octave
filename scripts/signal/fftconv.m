function c = fftconv (a, b, N)

  # usage:  fftconv (a, b [, N])
  #
  # c = fftconv (a, b) returns the convolution of the vectors a and b,
  # a vector with length equal to length (a) + length (b) - 1.  
  # If a and b are the coefficient vectors of two polynomials, c is
  # the coefficient vector of the product polynomial.
  #
  # The computation uses the FFT by calling fftfilt.  If the optional
  # argument N is specified, an N-point FFT is used.

  # Written by KH (Kurt.Hornik@ci.tuwien.ac.at) on Sep 3, 1994
  # Copyright Dept of Statistics and Probability Theory TU Wien
  
  if (nargin < 2 || nargin > 3)
    error ("usage:  fftconv (b, x [, N])");
  endif
  
  if (is_matrix (a) || is_matrix (b))
    error ("fftconv:  both a and b should be vectors");
  endif
  la = length (a);
  lb = length (b);
  if ((la == 1) || (lb == 1))
    c = a * b;
  else
    lc = la + lb - 1;
    a(lc) = 0;
    b(lc) = 0;
    if (nargin == 2)
      c = fftfilt (a, b);
    else
      if !(is_scalar (N))
	error ("fftconv:  N has to be a scalar");
      endif
      c = fftfilt (a, b, N);
    endif
  endif

endfunction
