
#ifndef __MODEL_H__
#define __MODEL_H__

#include "base.h"
#include "graphics.h"
#include "resmanager.h"
#include "application.h"

class CModel
{
public:
	typedef struct
	{
		vector3 vPosition;
		vector3 vNormal;
		vector2 vTexCoord0;
		vector3 vTangent;
	} vertexformat;

protected:
	// see CResManager
	friend void *pLoadModel( CResManager *i_pParent, string i_sFilename );
	friend void UnloadModel( CResManager *i_pParent, void *i_pResource );

	CModel( CResManager *i_pParent ) :
		m_pParent( i_pParent ), m_iNumFaces( 0 ), m_pVertexFormat( 0 ), m_pVertexBuffer( 0 )
	{
		CMuli3DDevice *pDevice = m_pParent->pGetParent()->pGetGraphics()->pGetM3DDevice();

		m3dvertexelement VertexDeclaration[] =
		{
			M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
			M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 1 ),
			M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 2 ),
			M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 3 ),
		};

		pDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) );
	}

	~CModel()
	{
		SAFE_RELEASE( m_pVertexBuffer );
		SAFE_RELEASE( m_pVertexFormat );
	}

	bool bLoadModel( const char *i_pData )
	{
		vector<vector3> Positions;
		vector<vector3> Normals;
		vector<vector2> TexCoords;
		vector<vertexformat> FinalVertices;

		const char *pCurPos = i_pData;
		while( *pCurPos )
		{
			switch( *pCurPos )
			{
			case 'v':
				{
					pCurPos++;

					switch( *pCurPos++ )
					{
					case ' ': // position
						{
							vector3 vPos;
							sscanf( pCurPos, "%f %f %f", &vPos.x, &vPos.y, &vPos.z );
							vPos.z = -vPos.z;
							Positions.push_back( vPos );
						}
						break;
					case 'n': // normal
						{
							vector3 vNormal;
							sscanf( pCurPos, "%f %f %f", &vNormal.x, &vNormal.y, &vNormal.z );
							vNormal.z = -vNormal.z;
							Normals.push_back( vNormal );
						}
						break;
					case 't': // texcoord
						{
							vector2 vTexCoord;
							sscanf( pCurPos, "%f %f", &vTexCoord.x, &vTexCoord.y );
							vTexCoord.y = 1 - vTexCoord.y;
							TexCoords.push_back( vTexCoord );
						}
						break;
					default:
						break;
					}
				}
				break;

			case 'f':
				{
					pCurPos++;

					uint32 iPosIndices[3], iNormalIndices[3], iTexIndices[3];
					sscanf( pCurPos, "%i/%i/%i %i/%i/%i %i/%i/%i",
						&iPosIndices[0], &iTexIndices[0], &iNormalIndices[0],
						&iPosIndices[1], &iTexIndices[1], &iNormalIndices[1],
						&iPosIndices[2], &iTexIndices[2], &iNormalIndices[2] );

					vertexformat NewVertex[3];
					for( uint32 iVertex = 0; iVertex < 3; ++iVertex )
					{
						NewVertex[iVertex].vPosition = Positions[iPosIndices[iVertex] - 1];
						NewVertex[iVertex].vNormal = Normals[iNormalIndices[iVertex] - 1];
						NewVertex[iVertex].vTexCoord0 = TexCoords[iTexIndices[iVertex] - 1];
					}

					// Calculate triangle tangent vector ----------------------
					// TODO: average tangents of shared vertices ...
					const vector3 vDelta[2] = { NewVertex[1].vPosition - NewVertex[0].vPosition,
						NewVertex[2].vPosition - NewVertex[0].vPosition };
					const float32 fDeltaV[2] = { NewVertex[1].vTexCoord0.y - NewVertex[0].vTexCoord0.y,
						NewVertex[2].vTexCoord0.y - NewVertex[0].vTexCoord0.y };
					const vector3 vTangent = ( vDelta[0] * fDeltaV[1] - vDelta[1] * fDeltaV[0] ).normalize();
					NewVertex[0].vTangent = NewVertex[1].vTangent = NewVertex[2].vTangent = vTangent;

					FinalVertices.push_back( NewVertex[2] );
					FinalVertices.push_back( NewVertex[1] );
					FinalVertices.push_back( NewVertex[0] );
					++m_iNumFaces;
				}
				break;
			default:
				break;
			}

			// advance to next line ...
			while( *pCurPos != 0 )
			{
				if( *pCurPos++ == '\n' )
					break;
			}
		}

		// Fill the vertex buffer ---------------------------------------------
		CMuli3DDevice *pDevice = m_pParent->pGetParent()->pGetGraphics()->pGetM3DDevice();
		if( FUNC_FAILED( pDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * (uint32)FinalVertices.size() ) ) )
			return false;

		vertexformat *pDest = 0;
		if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDest ) ) )
			return false;

		for( vector<vertexformat>::iterator pVertex = FinalVertices.begin(); pVertex != FinalVertices.end(); ++pVertex, ++pDest )
			memcpy( pDest, (vertexformat *)&(*pVertex), sizeof( vertexformat ) );

		return true;
	}

private:

public:
	inline CResManager *pGetParent() { return m_pParent; }

	inline uint32 iGetNumFaces() { return m_iNumFaces; }
	inline uint32 iGetNumVertices() { return 3 * m_iNumFaces; }
	inline uint32 iGetStride() { return sizeof( vertexformat ); }
	inline CMuli3DVertexFormat *pGetVertexFormat() { return m_pVertexFormat; }
	inline CMuli3DVertexBuffer *pGetVertexBuffer() { return m_pVertexBuffer; }

private:
	CResManager			*m_pParent;

	uint32				m_iNumFaces;
	CMuli3DVertexFormat	*m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
};

#endif // __MODEL_H__
