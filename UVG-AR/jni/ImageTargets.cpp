/*==============================================================================
Copyright (c) 2010-2013 QUALCOMM Austria Research Center GmbH.
All Rights Reserved.

@file 
    ImageTargets.cpp

@brief
    Sample for ImageTargets

==============================================================================*/


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef USE_OPENGL_ES_1_1
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/TrackableResult.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/DataSet.h>


#include "SampleUtils.h"
#include "Texture.h"
#include "CubeShaders.h"
#include "Teapot.h"
//#include "untitled1.h"
//#include "untitled2.h"
//#include "untitled3.h"
#include "untitled.h"
#include "banana.h"
#include "A101.h"
#include "Banco.h"
#include "Aula.h"
#include "EdificioA.h"
#include "Biblioteca.h"
#include "Dispensadoras.h"
#include "DesechosEl.h"
#include "UVG.h"


#ifdef __cplusplus
extern "C"
{
#endif

// Textures:
int textureCount                = 0;
Texture** textures              = 0;

// OpenGL ES 2.0 specific:
#ifdef USE_OPENGL_ES_2_0
unsigned int shaderProgramID    = 0;
GLint vertexHandle              = 0;
GLint normalHandle              = 0;
GLint textureCoordHandle        = 0;
GLint mvpMatrixHandle           = 0;
GLint texSampler2DHandle        = 0;
#endif

// Screen dimensions:
unsigned int screenWidth        = 0;
unsigned int screenHeight       = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
bool isActivityInPortraitMode   = false;

// The projection matrix used for rendering virtual objects:
QCAR::Matrix44F projectionMatrix;

// Constants:
static const float kObjectScale = 100.0f;

QCAR::DataSet* dataSetStonesAndChips    = 0;
QCAR::DataSet* dataSetTarmac            = 0;

bool switchDataSetAsap          = false;

// Object to receive update callbacks from QCAR SDK
class ImageTargets_UpdateCallback : public QCAR::UpdateCallback
{   
    virtual void QCAR_onUpdate(QCAR::State& /*state*/)
    {
        if (switchDataSetAsap)
        {
            switchDataSetAsap = false;

            // Get the image tracker:
            QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
            QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
            if (imageTracker == 0 || dataSetStonesAndChips == 0 || dataSetTarmac == 0 ||
                imageTracker->getActiveDataSet() == 0)
            {
                LOG("Failed to switch data set.");
                return;
            }
            
            if (imageTracker->getActiveDataSet() == dataSetStonesAndChips)
            {
                imageTracker->deactivateDataSet(dataSetStonesAndChips);
                imageTracker->activateDataSet(dataSetTarmac);
            }
            else
            {
                imageTracker->deactivateDataSet(dataSetTarmac);
                imageTracker->activateDataSet(dataSetStonesAndChips);
            }
        }
    }
};

ImageTargets_UpdateCallback updateCallback;

JNIEXPORT int JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_getOpenGlEsVersionNative(JNIEnv *, jobject)
{
#ifdef USE_OPENGL_ES_1_1        
    return 1;
#else
    return 2;
#endif
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
{
    isActivityInPortraitMode = isPortrait;
}



JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_switchDatasetAsap(JNIEnv *, jobject)
{
    switchDataSetAsap = true;
}


JNIEXPORT int JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initTracker(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initTracker");
    
    // Initialize the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* tracker = trackerManager.initTracker(QCAR::Tracker::IMAGE_TRACKER);
    if (tracker == NULL)
    {
        LOG("Failed to initialize ImageTracker.");
        return 0;
    }

    LOG("Successfully initialized ImageTracker.");
    return 1;
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitTracker(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitTracker");

    // Deinit the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    trackerManager.deinitTracker(QCAR::Tracker::IMAGE_TRACKER);
}


JNIEXPORT int JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_loadTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_loadTrackerData");
    
    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
                    trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL)
    {
        LOG("Failed to load tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }

    // Create the data sets:
    dataSetStonesAndChips = imageTracker->createDataSet();
    if (dataSetStonesAndChips == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }

    dataSetTarmac = imageTracker->createDataSet();
    if (dataSetTarmac == 0)
    {
        LOG("Failed to create a new tracking data.");
        return 0;
    }

    // Load the data sets:
    if (!dataSetStonesAndChips->load("AlgoritmosBD1.xml", QCAR::DataSet::STORAGE_APPRESOURCE))
    {
        LOG("Failed to load data set.");
        return 0;
    }

    if (!dataSetTarmac->load("Tarmac.xml", QCAR::DataSet::STORAGE_APPRESOURCE))
    {
        LOG("Failed to load data set.");
        return 0;
    }

    // Activate the data set:
    if (!imageTracker->activateDataSet(dataSetStonesAndChips))
    {
        LOG("Failed to activate data set.");
        return 0;
    }

    LOG("Successfully loaded and activated data set.");
    return 1;
}


JNIEXPORT int JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_destroyTrackerData(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_destroyTrackerData");

    // Get the image tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
        trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
    if (imageTracker == NULL)
    {
        LOG("Failed to destroy the tracking data set because the ImageTracker has not"
            " been initialized.");
        return 0;
    }
    
    if (dataSetStonesAndChips != 0)
    {
        if (imageTracker->getActiveDataSet() == dataSetStonesAndChips &&
            !imageTracker->deactivateDataSet(dataSetStonesAndChips))
        {
            LOG("Failed to destroy the tracking data set StonesAndChips because the data set "
                "could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(dataSetStonesAndChips))
        {
            LOG("Failed to destroy the tracking data set StonesAndChips.");
            return 0;
        }

        LOG("Successfully destroyed the data set StonesAndChips.");
        dataSetStonesAndChips = 0;
    }

    if (dataSetTarmac != 0)
    {
        if (imageTracker->getActiveDataSet() == dataSetTarmac &&
            !imageTracker->deactivateDataSet(dataSetTarmac))
        {
            LOG("Failed to destroy the tracking data set Tarmac because the data set "
                "could not be deactivated.");
            return 0;
        }

        if (!imageTracker->destroyDataSet(dataSetTarmac))
        {
            LOG("Failed to destroy the tracking data set Tarmac.");
            return 0;
        }

        LOG("Successfully destroyed the data set Tarmac.");
        dataSetTarmac = 0;
    }

    return 1;
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_onQCARInitializedNative(JNIEnv* env, jobject obj)
{
    // Register the update callback where we handle the data set swap:
    QCAR::registerCallback(&updateCallback);

    // Comment in to enable tracking of up to 2 targets simultaneously and
    // split the work over multiple frames:
    // QCAR::setHint(QCAR::HINT_MAX_SIMULTANEOUS_IMAGE_TARGETS, 2);
}


//int lastTrackableId = -1;
JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_renderFrame(JNIEnv *, jobject)
{

    //LOG("Java_com_qualcomm_QCARSamples_ImageTargets_GLRenderer_renderFrame");

    // Clear color and depth buffer 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Get the state from QCAR and mark the beginning of a rendering section
    QCAR::State state = QCAR::Renderer::getInstance().begin();
    
    // Explicitly render the Video Background
    QCAR::Renderer::getInstance().drawVideoBackground();
       
#ifdef USE_OPENGL_ES_1_1
    // Set GL11 flags:
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
        
#endif

    glEnable(GL_DEPTH_TEST);

    // We must detect if background reflection is active and adjust the culling direction. 
    // If the reflection is active, this means the post matrix has been reflected as well,
    // therefore standard counter clockwise face culling will result in "inside out" models. 
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    if(QCAR::Renderer::getInstance().getVideoBackgroundConfig().mReflection == QCAR::VIDEO_BACKGROUND_REFLECTION_ON)
        glFrontFace(GL_CW);  //Front camera
    else
        glFrontFace(GL_CCW);   //Back camera


    // Did we find any trackables this frame?
    for(int tIdx = 0; tIdx < state.getNumTrackableResults(); tIdx++)
    {
        // Get the trackable:
        const QCAR::TrackableResult* result = state.getTrackableResult(tIdx);
        const QCAR::Trackable& trackable = result->getTrackable();
/*
        if (trackable.getId() != lastTrackableId)
        {
			jstring js = env->NewStringUTF(trackable.getName());
			jclass javaClass = env->GetObjectClass(obj);
			jmethodID method = env->GetMethodID(javaClass, "displayMessage", "(Ljava/lang/String;)V");
			env->CallVoidMethod(obj, method, js);
			lastTrackableId = trackable.getId();
        }
*/

        QCAR::Matrix44F modelViewMatrix =
            QCAR::Tool::convertPose2GLMatrix(result->getPose());

        // Choose the texture based on the target name:
        int textureIndex;
        if (strcmp(trackable.getName(), "A101") == 0)
        {
            textureIndex = 0;
            const Texture* const thisTexture = textures[textureIndex];

#ifdef USE_OPENGL_ES_1_1
        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.f, 0.f, kObjectScale);
        glRotatef(90,0,1,0);
		glRotatef(90,1,0,0);
		glRotatef(90,0,0,1);
        glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

        // Draw object:
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) A101Verts);
        //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
        glDrawArrays(GL_TRIANGLES, 0, A101NumVerts);
#else

        QCAR::Matrix44F modelViewProjection;

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                     &modelViewMatrix.data[0]);
        SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
        		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
        		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                    &modelViewMatrix.data[0] ,
                                    &modelViewProjection.data[0]);

        glUseProgram(shaderProgramID);
         
        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) A101Verts);
        //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaNormals);
        //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaTexCoords);
        
        glEnableVertexAttribArray(vertexHandle);
        //glEnableVertexAttribArray(normalHandle);
        //glEnableVertexAttribArray(textureCoordHandle);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );
        glDrawArrays(GL_TRIANGLES, 0, A101NumVerts);

        glDisableVertexAttribArray(vertexHandle);
        //glDisableVertexAttribArray(normalHandle);
        //glDisableVertexAttribArray(textureCoordHandle);

        SampleUtils::checkGlError("ImageTargets renderFrame");
#endif

        }
        else if (strcmp(trackable.getName(), "AsociacionEstudiantes") == 0)
        {
            textureIndex = 0;
            const Texture* const thisTexture = textures[textureIndex];

#ifdef USE_OPENGL_ES_1_1
        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.f, 0.f, kObjectScale);
        glRotatef(90,0,1,0);
		glRotatef(90,1,0,0);
		glRotatef(90,0,0,1);
        glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

        // Draw object:
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) AulaVerts);
        //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
        glDrawArrays(GL_TRIANGLES, 0, AulaNumVerts);
#else

        QCAR::Matrix44F modelViewProjection;

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                     &modelViewMatrix.data[0]);
        SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
        		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
        		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                    &modelViewMatrix.data[0] ,
                                    &modelViewProjection.data[0]);

        glUseProgram(shaderProgramID);

        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) AulaVerts);
        //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaNormals);
        //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaTexCoords);

        glEnableVertexAttribArray(vertexHandle);
        //glEnableVertexAttribArray(normalHandle);
        //glEnableVertexAttribArray(textureCoordHandle);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );
        glDrawArrays(GL_TRIANGLES, 0, AulaNumVerts);

        glDisableVertexAttribArray(vertexHandle);
        //glDisableVertexAttribArray(normalHandle);
        //glDisableVertexAttribArray(textureCoordHandle);

        SampleUtils::checkGlError("ImageTargets renderFrame");
#endif



        }
        else if (strcmp(trackable.getName(), "Banco_Internacional") == 0)
		{
			textureIndex = 0;
			const Texture* const thisTexture = textures[textureIndex];

#ifdef USE_OPENGL_ES_1_1
        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.f, 0.f, kObjectScale);
        glRotatef(90,0,1,0);
		glRotatef(90,1,0,0);
		glRotatef(90,0,0,1);
        glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

        // Draw object:
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) BancoVerts);
        //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
        glDrawArrays(GL_TRIANGLES, 0, BancoNumVerts);
#else

        QCAR::Matrix44F modelViewProjection;

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                     &modelViewMatrix.data[0]);
        SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
        		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
        		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                    &modelViewMatrix.data[0] ,
                                    &modelViewProjection.data[0]);

        glUseProgram(shaderProgramID);

        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) BancoVerts);
        //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaNormals);
        //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaTexCoords);

        glEnableVertexAttribArray(vertexHandle);
        //glEnableVertexAttribArray(normalHandle);
        //glEnableVertexAttribArray(textureCoordHandle);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );
        glDrawArrays(GL_TRIANGLES, 0, BancoNumVerts);

        glDisableVertexAttribArray(vertexHandle);
        //glDisableVertexAttribArray(normalHandle);
        //glDisableVertexAttribArray(textureCoordHandle);

        SampleUtils::checkGlError("ImageTargets renderFrame");
#endif

		}
        else if (strcmp(trackable.getName(), "UVGectronicos") == 0)
        		{
        			textureIndex = 0;
        			const Texture* const thisTexture = textures[textureIndex];

        #ifdef USE_OPENGL_ES_1_1
                // Load projection matrix:
                glMatrixMode(GL_PROJECTION);
                glLoadMatrixf(projectionMatrix.data);

                // Load model view matrix:
                glMatrixMode(GL_MODELVIEW);
                glLoadMatrixf(modelViewMatrix.data);
                glTranslatef(0.f, 0.f, kObjectScale);
                glRotatef(90,0,1,0);
        		glRotatef(90,1,0,0);
        		glRotatef(90,0,0,1);
                glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

                // Draw object:
                glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
                glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) DesechosElVerts);
                //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
                glDrawArrays(GL_TRIANGLES, 0, DesechosElNumVerts);
        #else

                QCAR::Matrix44F modelViewProjection;

                SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                                 &modelViewMatrix.data[0]);
                SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                             &modelViewMatrix.data[0]);
                SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
                		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
                		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
                SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                            &modelViewMatrix.data[0] ,
                                            &modelViewProjection.data[0]);

                glUseProgram(shaderProgramID);

                glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                                      (const GLvoid*) DesechosElVerts);
                //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
                //                      (const GLvoid*) bananaNormals);
                //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
                //                      (const GLvoid*) bananaTexCoords);

                glEnableVertexAttribArray(vertexHandle);
                //glEnableVertexAttribArray(normalHandle);
                //glEnableVertexAttribArray(textureCoordHandle);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
                glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                                   (GLfloat*)&modelViewProjection.data[0] );
                glDrawArrays(GL_TRIANGLES, 0, DesechosElNumVerts);

                glDisableVertexAttribArray(vertexHandle);
                //glDisableVertexAttribArray(normalHandle);
                //glDisableVertexAttribArray(textureCoordHandle);

                SampleUtils::checkGlError("ImageTargets renderFrame");
        #endif

        		}

        else if (strcmp(trackable.getName(), "Dispensadoras") == 0)
                		{
                			textureIndex = 0;
                			const Texture* const thisTexture = textures[textureIndex];

                #ifdef USE_OPENGL_ES_1_1
                        // Load projection matrix:
                        glMatrixMode(GL_PROJECTION);
                        glLoadMatrixf(projectionMatrix.data);

                        // Load model view matrix:
                        glMatrixMode(GL_MODELVIEW);
                        glLoadMatrixf(modelViewMatrix.data);
                        glTranslatef(0.f, 0.f, kObjectScale);
                        glRotatef(90,0,1,0);
                		glRotatef(90,1,0,0);
                		glRotatef(90,0,0,1);
                        glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

                        // Draw object:
                        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                        //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
                        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) DispensadorasVerts);
                        //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
                        glDrawArrays(GL_TRIANGLES, 0, DispensadorasNumVerts);
                #else

                        QCAR::Matrix44F modelViewProjection;

                        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                                         &modelViewMatrix.data[0]);
                        SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                                     &modelViewMatrix.data[0]);
                        SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
                        		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
                        		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
                        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                                    &modelViewMatrix.data[0] ,
                                                    &modelViewProjection.data[0]);

                        glUseProgram(shaderProgramID);

                        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                                              (const GLvoid*) DispensadorasVerts);
                        //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
                        //                      (const GLvoid*) bananaNormals);
                        //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
                        //                      (const GLvoid*) bananaTexCoords);

                        glEnableVertexAttribArray(vertexHandle);
                        //glEnableVertexAttribArray(normalHandle);
                        //glEnableVertexAttribArray(textureCoordHandle);

                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                        glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
                        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                                           (GLfloat*)&modelViewProjection.data[0] );
                        glDrawArrays(GL_TRIANGLES, 0, DispensadorasNumVerts);

                        glDisableVertexAttribArray(vertexHandle);
                        //glDisableVertexAttribArray(normalHandle);
                        //glDisableVertexAttribArray(textureCoordHandle);

                        SampleUtils::checkGlError("ImageTargets renderFrame");
                #endif

                		}

        else if (strcmp(trackable.getName(), "BibliotecaUVG") == 0)
                        		{
                        			textureIndex = 0;
                        			const Texture* const thisTexture = textures[textureIndex];

                        #ifdef USE_OPENGL_ES_1_1
                                // Load projection matrix:
                                glMatrixMode(GL_PROJECTION);
                                glLoadMatrixf(projectionMatrix.data);

                                // Load model view matrix:
                                glMatrixMode(GL_MODELVIEW);
                                glLoadMatrixf(modelViewMatrix.data);
                                glTranslatef(0.f, 0.f, kObjectScale);
                                glRotatef(90,0,1,0);
                        		glRotatef(90,1,0,0);
                        		glRotatef(90,0,0,1);
                                glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

                                // Draw object:
                                glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                                //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
                                glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) UVGVerts);
                                //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
                                glDrawArrays(GL_TRIANGLES, 0, UVGNumVerts);
                        #else

                                QCAR::Matrix44F modelViewProjection;

                                SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                                                 &modelViewMatrix.data[0]);
                                SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                                             &modelViewMatrix.data[0]);
                                SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
                                		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
                                		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
                                SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                                            &modelViewMatrix.data[0] ,
                                                            &modelViewProjection.data[0]);

                                glUseProgram(shaderProgramID);

                                glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                                                      (const GLvoid*) UVGVerts);
                                //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
                                //                      (const GLvoid*) bananaNormals);
                                //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
                                //                      (const GLvoid*) bananaTexCoords);

                                glEnableVertexAttribArray(vertexHandle);
                                //glEnableVertexAttribArray(normalHandle);
                                //glEnableVertexAttribArray(textureCoordHandle);

                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                                glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
                                glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                                                   (GLfloat*)&modelViewProjection.data[0] );
                                glDrawArrays(GL_TRIANGLES, 0, UVGNumVerts);

                                glDisableVertexAttribArray(vertexHandle);
                                //glDisableVertexAttribArray(normalHandle);
                                //glDisableVertexAttribArray(textureCoordHandle);

                                SampleUtils::checkGlError("ImageTargets renderFrame");
                        #endif

                        		}

        else if (strcmp(trackable.getName(), "Dispensadores2") == 0)
                        		{
                        			textureIndex = 0;
                        			const Texture* const thisTexture = textures[textureIndex];

                        #ifdef USE_OPENGL_ES_1_1
                                // Load projection matrix:
                                glMatrixMode(GL_PROJECTION);
                                glLoadMatrixf(projectionMatrix.data);

                                // Load model view matrix:
                                glMatrixMode(GL_MODELVIEW);
                                glLoadMatrixf(modelViewMatrix.data);
                                glTranslatef(0.f, 0.f, kObjectScale);
                                glRotatef(90,0,1,0);
                        		glRotatef(90,1,0,0);
                        		glRotatef(90,0,0,1);
                                glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);

                                // Draw object:
                                glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                                //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
                                glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) DispensadorasVerts);
                                //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
                                glDrawArrays(GL_TRIANGLES, 0, DispensadorasNumVerts);
                        #else

                                QCAR::Matrix44F modelViewProjection;

                                SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                                                 &modelViewMatrix.data[0]);
                                SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                                             &modelViewMatrix.data[0]);
                                SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
                                		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
                                		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
                                SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                                            &modelViewMatrix.data[0] ,
                                                            &modelViewProjection.data[0]);

                                glUseProgram(shaderProgramID);

                                glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                                                      (const GLvoid*) DispensadorasVerts);
                                //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
                                //                      (const GLvoid*) bananaNormals);
                                //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
                                //                      (const GLvoid*) bananaTexCoords);

                                glEnableVertexAttribArray(vertexHandle);
                                //glEnableVertexAttribArray(normalHandle);
                                //glEnableVertexAttribArray(textureCoordHandle);

                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
                                glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
                                glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                                                   (GLfloat*)&modelViewProjection.data[0] );
                                glDrawArrays(GL_TRIANGLES, 0, DispensadorasNumVerts);

                                glDisableVertexAttribArray(vertexHandle);
                                //glDisableVertexAttribArray(normalHandle);
                                //glDisableVertexAttribArray(textureCoordHandle);

                                SampleUtils::checkGlError("ImageTargets renderFrame");
                        #endif

                        		}

        else if (strcmp(trackable.getName(), "EdificioA"))
		{
			textureIndex = 0;
			const Texture* const thisTexture = textures[textureIndex];

#ifdef USE_OPENGL_ES_1_1
        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.f, 0.f, kObjectScale);
        glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);
        glRotatef(90,0,1,0);
		glRotatef(90,1,0,0);
		glRotatef(90,0,0,1);

        // Draw object:
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        //glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) EdificioAVerts);
        //glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
        glDrawArrays(GL_TRIANGLES, 0, EdificioANumVerts);
#else

        QCAR::Matrix44F modelViewProjection;

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                     &modelViewMatrix.data[0]);
        SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
		SampleUtils::rotatePoseMatrix(90,1,0,0,&modelViewMatrix.data[0]);
		SampleUtils::rotatePoseMatrix(90,0,0,1,&modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                    &modelViewMatrix.data[0] ,
                                    &modelViewProjection.data[0]);

        glUseProgram(shaderProgramID);

        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) EdificioAVerts);
        //glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaNormals);
        //glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
        //                      (const GLvoid*) bananaTexCoords);

        glEnableVertexAttribArray(vertexHandle);
        //glEnableVertexAttribArray(normalHandle);
        //glEnableVertexAttribArray(textureCoordHandle);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );
        glDrawArrays(GL_TRIANGLES, 0, EdificioANumVerts);

        glDisableVertexAttribArray(vertexHandle);
        //glDisableVertexAttribArray(normalHandle);
        //glDisableVertexAttribArray(textureCoordHandle);

        SampleUtils::checkGlError("ImageTargets renderFrame");
#endif

		}


        else
        {
            textureIndex = 0;
            const Texture* const thisTexture = textures[textureIndex];

#ifdef USE_OPENGL_ES_1_1
        // Load projection matrix:
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix.data);

        // Load model view matrix:
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(modelViewMatrix.data);
        glTranslatef(0.f, 0.f, kObjectScale);
        glScalef(kObjectScale*10, kObjectScale*10, kObjectScale*10);
        glRotatef(90,0,1,0);
        glRotatef(180,1,0,0);
        glRotatef(-360,0,0,1);

        // Draw object:
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glTexCoordPointer(2, GL_FLOAT, 0, (const GLvoid*) bananaTexCoords);
        glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*) bananaVerts);
        glNormalPointer(GL_FLOAT, 0,  (const GLvoid*) bananaNormals);
        glDrawArrays(GL_TRIANGLES, 0, bananaNumVerts);
#else

        QCAR::Matrix44F modelViewProjection;

        SampleUtils::translatePoseMatrix(0.0f, 0.0f, kObjectScale,
                                         &modelViewMatrix.data[0]);
        SampleUtils::scalePoseMatrix(kObjectScale*10, kObjectScale*10, kObjectScale*10,
                                     &modelViewMatrix.data[0]);
        SampleUtils::rotatePoseMatrix(90,0,1,0,&modelViewMatrix.data[0]);
		SampleUtils::rotatePoseMatrix(180,1,0,0,&modelViewMatrix.data[0]);
		SampleUtils::rotatePoseMatrix(-360,0,0,1,&modelViewMatrix.data[0]);
        SampleUtils::multiplyMatrix(&projectionMatrix.data[0],
                                    &modelViewMatrix.data[0] ,
                                    &modelViewProjection.data[0]);

        glUseProgram(shaderProgramID);

        glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) bananaVerts);
        glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) bananaNormals);
        glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
                              (const GLvoid*) bananaTexCoords);

        glEnableVertexAttribArray(vertexHandle);
        glEnableVertexAttribArray(normalHandle);
        glEnableVertexAttribArray(textureCoordHandle);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);
        glUniform1i(texSampler2DHandle, 0 /*GL_TEXTURE0*/);
        glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
                           (GLfloat*)&modelViewProjection.data[0] );
        glDrawArrays(GL_TRIANGLES, 0, bananaNumVerts);

        glDisableVertexAttribArray(vertexHandle);
        glDisableVertexAttribArray(normalHandle);
        glDisableVertexAttribArray(textureCoordHandle);

        SampleUtils::checkGlError("ImageTargets renderFrame");
#endif


        }


    }

    glDisable(GL_DEPTH_TEST);

#ifdef USE_OPENGL_ES_1_1        
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

    QCAR::Renderer::getInstance().end();
}


void
configureVideoBackground()
{
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.
                                getVideoMode(QCAR::CameraDevice::MODE_DEFAULT);


    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;
    
    if (isActivityInPortraitMode)
    {
        //LOG("configureVideoBackground PORTRAIT");
        config.mSize.data[0] = videoMode.mHeight
                                * (screenHeight / (float)videoMode.mWidth);
        config.mSize.data[1] = screenHeight;

        if(config.mSize.data[0] < screenWidth)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenWidth;
            config.mSize.data[1] = screenWidth * 
                              (videoMode.mWidth / (float)videoMode.mHeight);
        }
    }
    else
    {
        //LOG("configureVideoBackground LANDSCAPE");
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
                            * (screenWidth / (float)videoMode.mWidth);

        if(config.mSize.data[1] < screenHeight)
        {
            LOG("Correcting rendering background size to handle missmatch between screen and video aspect ratios.");
            config.mSize.data[0] = screenHeight
                                * (videoMode.mWidth / (float)videoMode.mHeight);
            config.mSize.data[1] = screenHeight;
        }
    }

    LOG("Configure Video Background : Video (%d,%d), Screen (%d,%d), mSize (%d,%d)", videoMode.mWidth, videoMode.mHeight, screenWidth, screenHeight, config.mSize.data[0], config.mSize.data[1]);

    // Set the config:
    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initApplicationNative(
                            JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initApplicationNative");
    
    // Store screen dimensions
    screenWidth = width;
    screenHeight = height;
        
    // Handle to the activity class:
    jclass activityClass = env->GetObjectClass(obj);

    jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
                                                    "getTextureCount", "()I");
    if (getTextureCountMethodID == 0)
    {
        LOG("Function getTextureCount() not found.");
        return;
    }

    textureCount = env->CallIntMethod(obj, getTextureCountMethodID);    
    if (!textureCount)
    {
        LOG("getTextureCount() returned zero.");
        return;
    }

    textures = new Texture*[textureCount];

    jmethodID getTextureMethodID = env->GetMethodID(activityClass,
        "getTexture", "(I)Lcom/qualcomm/QCARSamples/ImageTargets/Texture;");

    if (getTextureMethodID == 0)
    {
        LOG("Function getTexture() not found.");
        return;
    }

    // Register the textures
    for (int i = 0; i < textureCount; ++i)
    {

        jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, i); 
        if (textureObject == NULL)
        {
            LOG("GetTexture() returned zero pointer");
            return;
        }

        textures[i] = Texture::create(env, textureObject);
    }
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_initApplicationNative finished");
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitApplicationNative(
                                                        JNIEnv* env, jobject obj)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_deinitApplicationNative");

    // Release texture resources
    if (textures != 0)
    {    
        for (int i = 0; i < textureCount; ++i)
        {
            delete textures[i];
            textures[i] = NULL;
        }
    
        delete[]textures;
        textures = NULL;
        
        textureCount = 0;
    }
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_startCamera(JNIEnv *,
                                                                         jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_startCamera");
    
    // Select the camera to open, set this to QCAR::CameraDevice::CAMERA_FRONT 
    // to activate the front camera instead.
    QCAR::CameraDevice::CAMERA camera = QCAR::CameraDevice::CAMERA_DEFAULT;

    // Initialize the camera:
    if (!QCAR::CameraDevice::getInstance().init(camera))
        return;

    // Configure the video background
    configureVideoBackground();

    // Select the default mode:
    if (!QCAR::CameraDevice::getInstance().selectVideoMode(
                                QCAR::CameraDevice::MODE_DEFAULT))
        return;

    // Start the camera:
    if (!QCAR::CameraDevice::getInstance().start())
        return;

    // Uncomment to enable flash
    //if(QCAR::CameraDevice::getInstance().setFlashTorchMode(true))
    //    LOG("IMAGE TARGETS : enabled torch");

    // Uncomment to enable infinity focus mode, or any other supported focus mode
    // See CameraDevice.h for supported focus modes
    //if(QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_INFINITY))
    //    LOG("IMAGE TARGETS : enabled infinity focus");

    // Start the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->start();
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_stopCamera(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_stopCamera");

    // Stop the tracker:
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0)
        imageTracker->stop();
    
    QCAR::CameraDevice::getInstance().stop();
    QCAR::CameraDevice::getInstance().deinit();
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setProjectionMatrix(JNIEnv *, jobject)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setProjectionMatrix");

    // Cache the projection matrix:
    const QCAR::CameraCalibration& cameraCalibration =
                                QCAR::CameraDevice::getInstance().getCameraCalibration();
    projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f, 2500.0f);
}

// ----------------------------------------------------------------------------
// Activates Camera Flash
// ----------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_activateFlash(JNIEnv*, jobject, jboolean flash)
{
    return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_autofocus(JNIEnv*, jobject)
{
    return QCAR::CameraDevice::getInstance().setFocusMode(QCAR::CameraDevice::FOCUS_MODE_TRIGGERAUTO) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT jboolean JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargets_setFocusMode(JNIEnv*, jobject, jint mode)
{
    int qcarFocusMode;

    switch ((int)mode)
    {
        case 0:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
            break;
        
        case 1:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
            break;
            
        case 2:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
            break;
            
        case 3:
            qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
            break;
    
        default:
            return JNI_FALSE;
    }
    
    return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;
}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_initRendering(
                                                    JNIEnv* env, jobject obj)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_initRendering");

    // Define clear color
    glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);
    
    // Now generate the OpenGL texture objects and add settings
    for (int i = 0; i < textureCount; ++i)
    {
        glGenTextures(1, &(textures[i]->mTextureID));
        glBindTexture(GL_TEXTURE_2D, textures[i]->mTextureID);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[i]->mWidth,
                textures[i]->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                (GLvoid*)  textures[i]->mData);
    }
#ifndef USE_OPENGL_ES_1_1
  
    shaderProgramID     = SampleUtils::createProgramFromBuffer(cubeMeshVertexShader,
                                                            cubeFragmentShader);

    vertexHandle        = glGetAttribLocation(shaderProgramID,
                                                "vertexPosition");
    normalHandle        = glGetAttribLocation(shaderProgramID,
                                                "vertexNormal");
    textureCoordHandle  = glGetAttribLocation(shaderProgramID,
                                                "vertexTexCoord");
    mvpMatrixHandle     = glGetUniformLocation(shaderProgramID,
                                                "modelViewProjectionMatrix");
    texSampler2DHandle  = glGetUniformLocation(shaderProgramID, 
                                                "texSampler2D");
                                                
#endif

}


JNIEXPORT void JNICALL
Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_updateRendering(
                        JNIEnv* env, jobject obj, jint width, jint height)
{
    LOG("Java_com_qualcomm_QCARSamples_ImageTargets_ImageTargetsRenderer_updateRendering");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
}


#ifdef __cplusplus
}
#endif
