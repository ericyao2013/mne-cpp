//=============================================================================================================
/**
* @file     rtsourceinterpolationmatworker.cpp
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Lars Debor <lars.debor@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     June, 2017
*
* @section  LICENSE
*
* Copyright (C) 2017, Lorenz Esch, Lars Debor and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    RtSourceInterpolationMatWorker class definition.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "rtsourceinterpolationmatworker.h"
#include "../../../../helpers/geometryinfo/geometryinfo.h"
#include "../../../../helpers/interpolation/interpolation.h"


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace DISP3DLIB;
using namespace MNELIB;
using namespace Eigen;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

RtSourceInterpolationMatWorker::RtSourceInterpolationMatWorker()
: m_bInterpolationInfoIsInit(false)
{
    m_lInterpolationData.dCancelDistance = 0.05;
    m_lInterpolationData.interpolationFunction = DISP3DLIB::Interpolation::cubic;
}


//*************************************************************************************************************

void RtSourceInterpolationMatWorker::setInterpolationFunction(const QString &sInterpolationFunction)
{
    if(sInterpolationFunction == "Linear") {
        m_lInterpolationData.interpolationFunction = Interpolation::linear;
    }
    else if(sInterpolationFunction == "Square") {
        m_lInterpolationData.interpolationFunction = Interpolation::square;
    }
    else if(sInterpolationFunction == "Cubic") {
        m_lInterpolationData.interpolationFunction = Interpolation::cubic;
    }
    else if(sInterpolationFunction == "Gaussian") {
        m_lInterpolationData.interpolationFunction = Interpolation::gaussian;
    }

    if(m_bInterpolationInfoIsInit == true){
        //recalculate Interpolation matrix parameters changed
        SparseMatrix<float> matInterpolationMat = Interpolation::createInterpolationMat(m_lInterpolationData.vecMappedSubset,
                                                                                        m_lInterpolationData.matDistanceMatrix,
                                                                                        m_lInterpolationData.interpolationFunction,
                                                                                        m_lInterpolationData.dCancelDistance);

        emit newInterpolationMatrixCalculated(matInterpolationMat);
    }
}


//*************************************************************************************************************

void RtSourceInterpolationMatWorker::setCancelDistance(double dCancelDist)
{
    m_lInterpolationData.dCancelDistance = dCancelDist;

    //recalculate everything because parameters changed
    calculateInterpolationOperator();
}


//*************************************************************************************************************

void RtSourceInterpolationMatWorker::setInterpolationInfo(const Eigen::MatrixX3f &matVertices,
                                                          const QVector<QVector<int> > &vecNeighborVertices,
                                                          const QVector<qint32> &vecMappedSubset)
{
    if(matVertices.rows() == 0) {
        qDebug() << "RtSourceInterpolationMatWorker::calculateSurfaceData - Surface data is empty. Returning ...";
        return;
    }

    //set members
    m_lInterpolationData.matVertices = matVertices;
    m_lInterpolationData.vecNeighborVertices = vecNeighborVertices;
    m_lInterpolationData.vecMappedSubset = vecMappedSubset;

    m_bInterpolationInfoIsInit = true;

    calculateInterpolationOperator();
}


//*************************************************************************************************************

void RtSourceInterpolationMatWorker::calculateInterpolationOperator()
{
    if(!m_bInterpolationInfoIsInit) {
        qDebug() << "RtSourceInterpolationMatWorker::calculateInterpolationOperator - Set interpolation info first.";
        return;
    }

    //SCDC with cancel distance
    m_lInterpolationData.matDistanceMatrix = GeometryInfo::scdc(m_lInterpolationData.matVertices,
                                                                m_lInterpolationData.vecNeighborVertices,
                                                                m_lInterpolationData.vecMappedSubset,
                                                                m_lInterpolationData.dCancelDistance);

    //create Interpolation matrix
    SparseMatrix<float> matInterpolationMat = Interpolation::createInterpolationMat(m_lInterpolationData.vecMappedSubset,
                                                                                    m_lInterpolationData.matDistanceMatrix,
                                                                                    m_lInterpolationData.interpolationFunction,
                                                                                    m_lInterpolationData.dCancelDistance);

    emit newInterpolationMatrixCalculated(matInterpolationMat);
}
