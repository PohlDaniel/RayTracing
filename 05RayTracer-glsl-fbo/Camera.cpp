#include <cmath>
#include "camera.h"

const float Camera::DEFAULT_FOVX = 45.0f;
const float Camera::DEFAULT_ZFAR = 1000.0f;
const float Camera::DEFAULT_ZNEAR = 0.1f;
const Vector3f Camera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vector3f Camera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vector3f Camera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

Camera::Camera(){  

    m_fovx = DEFAULT_FOVX;
    m_znear = DEFAULT_ZNEAR;
    m_zfar = DEFAULT_ZFAR;
    m_aspectRatio = 0.0f;
    
    m_eye.set(0.0f, 0.0f, 0.0f);
    m_xAxis.set(1.0f, 0.0f, 0.0f);
    m_yAxis.set(0.0f, 1.0f, 0.0f);
    m_zAxis.set(0.0f, 0.0f, 1.0f);
    m_viewDir.set(0.0f, 0.0f, -1.0f);
    
    m_viewMatrix.identity();
    m_projMatrix.identity();
	m_orthMatrix.identity();
}

Camera::~Camera(){
}


void Camera::updateViewMatrix(bool orthogonalizeAxes)
{
    
     // Regenerate the camera's local axes to orthogonalize them.
     if (orthogonalizeAxes){

       m_zAxis.normalize();
        
       m_yAxis = Vector3f::cross(m_zAxis, m_xAxis);
       m_yAxis.normalize();
        
       m_xAxis = Vector3f::cross(m_yAxis, m_zAxis);
       m_xAxis.normalize();

       m_viewDir = -m_zAxis;
	}

    // Reconstruct the view matrix.

    m_viewMatrix[0][0] = m_xAxis.x;
    m_viewMatrix[1][0] = m_xAxis.y;
    m_viewMatrix[2][0] = m_xAxis.z;
    m_viewMatrix[3][0] = -Vector3f::dot(m_xAxis, m_eye);

    m_viewMatrix[0][1] = m_yAxis.x;
    m_viewMatrix[1][1] = m_yAxis.y;
    m_viewMatrix[2][1] = m_yAxis.z;
    m_viewMatrix[3][1] = -Vector3f::dot(m_yAxis, m_eye);

    m_viewMatrix[0][2] = m_zAxis.x;    
    m_viewMatrix[1][2] = m_zAxis.y;
    m_viewMatrix[2][2] = m_zAxis.z;   
    m_viewMatrix[3][2] = -Vector3f::dot(m_zAxis, m_eye);

    m_viewMatrix[0][3] = 0.0f;
    m_viewMatrix[1][3] = 0.0f;
    m_viewMatrix[2][3] = 0.0f;
    m_viewMatrix[3][3] = 1.0f;
}

void Camera::perspective(float fovx, float aspect, float znear, float zfar){

    // Construct a projection matrix based on the horizontal field of view
    // 'fovx' rather than the more traditional vertical field of view 'fovy'.

	float e = tanf(PI*fovx/360 );
	float xScale = (1/e)/aspect;
    float yScale = 1/e;

	

    m_projMatrix[0][0] = xScale;
    m_projMatrix[0][1] = 0.0f;
    m_projMatrix[0][2] = 0.0f;
    m_projMatrix[0][3] = 0.0f;

    m_projMatrix[1][0] = 0.0f;
    m_projMatrix[1][1] = yScale;
    m_projMatrix[1][2] = 0.0f;
    m_projMatrix[1][3] = 0.0f;

    m_projMatrix[2][0] = 0.0f;
    m_projMatrix[2][1] = 0.0f;
    m_projMatrix[2][2] = (zfar + znear) / (znear - zfar);
    m_projMatrix[2][3] = -1.0f;

    m_projMatrix[3][0] = 0.0f;
    m_projMatrix[3][1] = 0.0f;
    m_projMatrix[3][2] = (2.0f * zfar * znear) / (znear - zfar);
    m_projMatrix[3][3] = 0.0f;

    m_fovx = fovx;
    m_aspectRatio = aspect;
    m_znear = znear;
    m_zfar = zfar;
}


void Camera::orthographic(float left, float right, float bottom, float top, float znear, float zfar){

	m_orthMatrix[0][0] = 2/(right-left);
	m_orthMatrix[0][1] = 0.0f;
	m_orthMatrix[0][2] = 0.0f;
	m_orthMatrix[0][3] = 0.0f;

	m_orthMatrix[1][0] = 0.0f;
	m_orthMatrix[1][1] = 2/(top-bottom);
	m_orthMatrix[1][2] = 0.0f;
	m_orthMatrix[1][3] = 0.0f;

	m_orthMatrix[2][0] = 0.0f;
	m_orthMatrix[2][1] = 0.0f;
	m_orthMatrix[2][2] = 2 / (znear - zfar);
	m_orthMatrix[2][3] = 0.0f;

	m_orthMatrix[3][0] = (right + left)/(left - right);
	m_orthMatrix[3][1] = (top + bottom) / (bottom - top);
	m_orthMatrix[3][2] = (zfar + znear) / (znear - zfar);
	m_orthMatrix[3][3] = 1.0f;

	m_znear = znear;
	m_zfar = zfar;
}

void Camera::lookAt(const Vector3f &eye, const Vector3f &target, const Vector3f &up){

    m_eye = eye;

    m_zAxis = eye - target;
    m_zAxis.normalize();

    m_viewDir = -m_zAxis;

    m_xAxis = Vector3f::cross(up, m_zAxis);
    m_xAxis.normalize();

    m_yAxis = Vector3f::cross(m_zAxis, m_xAxis);
    m_yAxis.normalize();
    m_xAxis.normalize();

    m_viewMatrix[0][0] = m_xAxis.x;
    m_viewMatrix[1][0] = m_xAxis.y;
    m_viewMatrix[2][0] = m_xAxis.z;
    m_viewMatrix[3][0] = -Vector3f::dot(m_xAxis, eye);

    m_viewMatrix[0][1] = m_yAxis.x;
    m_viewMatrix[1][1] = m_yAxis.y;
    m_viewMatrix[2][1] = m_yAxis.z;
    m_viewMatrix[3][1] = -Vector3f::dot(m_yAxis, eye);

    m_viewMatrix[0][2] = m_zAxis.x;
    m_viewMatrix[1][2] = m_zAxis.y;
    m_viewMatrix[2][2] = m_zAxis.z;    
    m_viewMatrix[3][2] = -Vector3f::dot(m_zAxis, eye);

    // Extract the pitch angle from the view matrix.
    m_accumPitchDegrees = -asinf(m_viewMatrix[1][2])*180.f/PI;
}

void Camera::move(float dx, float dy, float dz){

    // Moves the camera by dx world units to the left or right; dy
    // world units upwards or downwards; and dz world units forwards
    // or backwards.

    Vector3f eye = m_eye;
    Vector3f forwards;

	forwards = Vector3f::cross(WORLD_YAXIS, m_xAxis);
	forwards.normalize();
   

    eye += m_xAxis * dx;
    eye += WORLD_YAXIS * dy;
    eye += forwards * dz;
    
    setPosition(eye);
}



void Camera::rotate(float yaw, float pitch){

    rotateFirstPerson(pitch, yaw);
    updateViewMatrix(true);
}

void Camera::rotateFirstPerson(float pitch, float yaw){

    m_accumPitchDegrees += pitch;

    if (m_accumPitchDegrees > 90.0f)
    {
        pitch = 90.0f - (m_accumPitchDegrees - pitch);
        m_accumPitchDegrees = 90.0f;
    }

    if (m_accumPitchDegrees < -90.0f)
    {
        pitch = -90.0f - (m_accumPitchDegrees - pitch);
        m_accumPitchDegrees = -90.0f;
    }

    Matrix4f rotMtx;

    // Rotate camera's existing x and z axes about the world y axis.
    if (yaw != 0.0f)
    {
        rotMtx.rotate(WORLD_YAXIS, yaw);
        m_xAxis = m_xAxis * rotMtx;
        m_zAxis = m_zAxis * rotMtx;
    }

    // Rotate camera's existing y and z axes about its existing x axis.
    if (pitch != 0.0f)
    {
        rotMtx.rotate(m_xAxis, pitch);
        m_yAxis = m_yAxis * rotMtx;
        m_zAxis = m_zAxis * rotMtx;
    }
}


void Camera::setPosition(float x, float y, float z){

    m_eye.set(x, y, z);
    updateViewMatrix(false);
}

void Camera::setPosition(const Vector3f &position){

    m_eye = position;
    updateViewMatrix(false);
}



const Matrix4f &Camera::getViewMatrix() const{ 

	return m_viewMatrix; 
}

 const Matrix4f &Camera::getProjectionMatrix() const{ 
	 
	 return m_projMatrix;
 }

 const Matrix4f &Camera::getOrthographicMatrix() const{

	 return m_orthMatrix;
 }

const Vector3f &Camera::getPosition() const{

	return m_eye;
}





