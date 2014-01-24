/*!
 * 
 * \file        CopyConst.h
 *
 * \brief       -
 *
 * \author      -
 * \date        -
 *
 *
 * \par Copyright 1995-2014 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://image.diku.dk/shark/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SHARK_DATA_CORE_COPYCONST_H
#define SHARK_DATA_CORE_COPYCONST_H

#include <boost/type_traits/remove_const.hpp>

namespace shark{

///\brief If U is a const Type, than T is also made const. 
template<class T, class U>
struct CopyConst{
	typedef typename boost::remove_const<T>::type type;
};
template<class T, class U>
struct CopyConst<T,U const>{
	typedef typename boost::remove_const<T>::type const type;
};


}

#endif

