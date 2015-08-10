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

#include "../../include/core/m3dcore.h"
#include "../../include/core/m3dcore_device.h"

CMuli3D::CMuli3D()
{}

CMuli3D::~CMuli3D()
{}

result CMuli3D::CreateDevice( CMuli3DDevice **o_ppDevice, const m3ddeviceparameters *i_pDeviceParameters )
{
	if( !o_ppDevice )
	{
		FUNC_FAILING( "CMuli3D::CreateDevice: parameter o_ppDevice points to null.\n" );
		return e_invalidparameters;
	}

	if( !i_pDeviceParameters )
	{
		*o_ppDevice = 0;
		FUNC_FAILING( "CMuli3D::CreateDevice: parameter i_pDeviceParameters points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppDevice = new CMuli3DDevice( this, i_pDeviceParameters );
	if( !(*o_ppDevice) )
	{
		FUNC_FAILING( "CMuli3D::CreateDevice: out of memory, cannot create device.\n" );
		return e_outofmemory;
	}

	return (*o_ppDevice)->Create();
}

// ----------------------------------------------------------------------------

result CreateMuli3D( CMuli3D **o_ppMuli3D )
{
	if( !o_ppMuli3D )
	{
		FUNC_FAILING( "CreateMuli3D: parameter o_ppMuli3D points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppMuli3D = new CMuli3D;
	if( !(*o_ppMuli3D) )
	{
		FUNC_FAILING( "CreateMuli3D: out of memory, cannot create Muli3D-instance.\n" );
		return e_outofmemory;
	}

	return s_ok;
};
