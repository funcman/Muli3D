
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "base.h"
#include "../../libmuli3d/include/m3d.h"

enum eVisibility
{
	eVisibility_CompletelyOut = 0,
	eVisibility_Partly,
	eVisibility_CompletelyIn
};

class CCamera
{
public:
	CCamera( class CGraphics *i_pParent );
	virtual ~CCamera();

	// Create rendersurface/depthsurface and replaces set ones+viewport with them -> after calling this you can't change surfaces anymore!
	bool bCreateRenderCamera( uint32 i_iWidth, uint32 i_iHeight, m3dformat i_fmtFrameBuffer = m3dfmt_r32g32b32f, bool i_bDepthBuffer = true );

	void CalculateProjection( float32 i_fFOVAngle, float32 i_fViewDistance, float32 i_fNearClippingPlane = 1.0f, float32 i_fAspect = 4.0f / 3.0f ); // Call this before calling CalculateView(), because the projection matrix is needed for frustum calcs!
	void CalculateView(); // Has to be called after making changes to camera position / rotation ...

	eVisibility eSphereVisible( const vector3 &i_vOrigin, float32 i_fRadius ); // Check does not support eVisibility_Partly
	eVisibility eBoxVisible( const vector3 &i_vLower, const vector3 &i_vUpper );

	void BeginRender();
	void ClearToSceneColor( const m3drect *i_pRect = 0 );

	// RenderPass sets the necessary states to render a specific pass and then calls the scene's render function
	virtual void RenderPass( int32 i_iPass = -1 ) = 0; // -1 = render all passes
	
	void EndRender( bool i_bPresentToScreen = false );

private:
	void BuildFrustum();

public:
	inline class CGraphics *pGetParent() { return m_pParent; }

	inline CMuli3DRenderTarget *pGetRenderTarget() { return m_pRenderTarget; }

	inline void SetWorldMatrix( const matrix44 &i_matWorld ) { m_matWorld = i_matWorld; }
	inline void SetViewMatrix( const matrix44 &i_matView ) { m_matView = i_matView; }
	inline void SetProjectionMatrix( const matrix44 &i_matProjection ) { m_matProjection = i_matProjection; }

	inline const matrix44 &matGetWorldMatrix() { return m_matWorld; }
	inline const matrix44 &matGetViewMatrix() { return m_matView; }
	inline const matrix44 &matGetProjectionMatrix() { return m_matProjection; }

	inline float32 fGetFOV() { return m_fFOVAngle; }
	inline float32 fGetAspect() { return m_fAspect; }
	inline float32 fGetNearClippingPlane() { return m_fNearClippingPlane; }
	inline float32 fGetViewDistance() { return m_fViewDistance; }

	inline void SetPosition( const vector3 &i_vPosition ) { m_vPosition = i_vPosition; }
	inline void SetPositionRel( const vector3 &i_vPositionRel ) { m_vPosition += i_vPositionRel; }
	inline const vector3 &vGetPosition() { return m_vPosition; }

	inline void SetRotation( const vector3 &i_vRotation ) { matrix44 matTemp; matMatrix44RotationYawPitchRoll( matTemp, i_vRotation ); qQuaternionRotationMatrix( m_qOrientation, matTemp ); }
	inline void SetRotationRel( const vector3 &i_vRotationRel ) { matrix44 matTemp; matMatrix44RotationYawPitchRoll( matTemp, i_vRotationRel ); quaternion qTemp; qQuaternionRotationMatrix( qTemp, matTemp ); m_qOrientation *= qTemp; }
	inline void SetLookAt( const vector3 &i_vPosition, const vector3 i_vUp ) { matrix44 matTemp; matMatrix44LookAtLH( matTemp, m_vPosition, i_vPosition, i_vUp ); qQuaternionRotationMatrix( m_qOrientation, matTemp ); }
	inline void SetOrientation( const quaternion &i_qOrientation ) { m_qOrientation = i_qOrientation; }
	inline const quaternion &qGetOrientation() { return m_qOrientation; }

	inline const vector3 &vGetDirection() { return m_vDir; }
	inline const vector3 &vGetRightVector() { return m_vRight; }
	inline const vector3 &vGetUpVector() { return m_vUp; }

public:
	class CGraphics *m_pParent;
	
	CMuli3DRenderTarget *m_pRenderTarget;
	bool m_bLockedSurfacesViewport;

	matrix44	m_matWorld, m_matView, m_matProjection;
	plane		m_plFrustum[6];

	float32		m_fFOVAngle, m_fAspect, m_fNearClippingPlane, m_fViewDistance;
	vector3		m_vPosition;
	quaternion	m_qOrientation;
	vector3		m_vUp, m_vRight, m_vDir;
};

#endif // __CAMERA_H__
