#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraTargetInitial = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, this->cameraUpDirection));
        this->cameraPositionInitial = cameraPosition;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        glm::mat4 transform;
        switch (direction) {
        case MOVE_FORWARD:
            transform = glm::translate(glm::mat4(1.0f), cameraFrontDirection * speed);
            //cameraTarget.z -= speed;
            //cameraPosition.z -= speed;
            break;
        case MOVE_BACKWARD:
            transform = glm::translate(glm::mat4(1.0f), (-cameraFrontDirection) * speed);
            //cameraTarget.z += speed;
            //cameraPosition.z += speed;
            break;
        case MOVE_RIGHT:
            transform = glm::translate(glm::mat4(1.0f), cameraRightDirection * speed);
            //cameraTarget.x += speed;
            //cameraPosition.x += speed;
            break;
        case MOVE_LEFT:
            transform = glm::translate(glm::mat4(1.0f), (-cameraRightDirection) * speed);
            //cameraTarget.x -= speed;
            //cameraPosition.x -= speed;
            break;
        case MOVE_UP:
            transform = glm::translate(glm::mat4(1.0f), cameraUpDirection * speed);
            //cameraTarget.y += speed;
            //cameraPosition.y += speed;
            break;
        case MOVE_DOWN:
            transform = glm::translate(glm::mat4(1.0f), (-cameraUpDirection) * speed);
            //cameraTarget.y -= speed;
            //cameraPosition.y -= speed;
            break;
        }
        cameraTarget = glm::vec3(transform * glm::vec4(cameraTarget, 1.0f));
        cameraPosition = glm::vec3(transform * glm::vec4(cameraPosition, 1.0f));
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), cameraPosition);
        transform = glm::rotate(transform, glm::radians(pitch), cameraUpDirection);
        transform = glm::translate(transform, -cameraPosition);

        cameraTarget = glm::vec3(transform * glm::vec4(cameraTargetInitial, 1.0f));
        cameraRightDirection = glm::normalize(glm::cross(glm::normalize(cameraTarget - cameraPosition), cameraUpDirection));

        transform = glm::translate(glm::mat4(1.0f), cameraPosition);
        transform = glm::rotate(transform, glm::radians(yaw), cameraRightDirection);
        transform = glm::rotate(transform, glm::radians(pitch), cameraUpDirection);
        transform = glm::translate(transform, -cameraPosition);

        cameraTarget = glm::vec3(transform * glm::vec4(cameraTargetInitial, 1.0f));
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        
    }
}