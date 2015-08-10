/*
	Muli3D - a software rendering library
	Copyright (C) 2004, 2005 Stephan Reiter <streiter@aon.at>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../../include/core/m3dcore_basetexture.h"
#include "../../include/core/m3dcore_device.h"

IMuli3DBaseTexture::IMuli3DBaseTexture( CMuli3DDevice *i_pParent ) :
	m_pParent( i_pParent )
{
	m_pParent->AddRef();
}

IMuli3DBaseTexture::~IMuli3DBaseTexture()
{
	SAFE_RELEASE( m_pParent );
}

CMuli3DDevice *IMuli3DBaseTexture::pGetDevice()
{
	if( m_pParent )
		m_pParent->AddRef();
	
	return m_pParent;
}
