#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software. 
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#

#
# Find the Teem library
#

#
# This module finds the Teem include files and libraries. 
# It also determines what the name of the library is. 
# This code sets the following variables:
#
# TEEM_LIBRARY the libraries to link against when using Teem
# TEEM_INCLUDE where to find teem/nrrd.h, etc.
#

# DV Fix finding teem library in thirdparty
FIND_LIBRARY(TEEM_LIBRARY teem
             PATHS /usr
)

# Teem should have been configured with zlib and png support.
IF(TEEM_LIBRARY)
  SET(TEEM_LIBRARY ${TEEM_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
ENDIF(TEEM_LIBRARY)

FIND_PATH(TEEM_INCLUDE_DIR teem/nrrd.h
          ${SCIRUN_THIRDPARTY_DIR}/include
)
