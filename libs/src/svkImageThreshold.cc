/*
 *  Copyright © 2009-2014 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice, 
 *      this list of conditions and the following disclaimer in the documentation 
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name 
 *      "The Regents of the University of California," or the names of any of its 
 *      contributors may be used to endorse or promote products derived from this 
 *      software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *  OF SUCH DAMAGE.
 */



/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkImageThreshold.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageThreshold, "$Rev$");
vtkStandardNewMacro(svkImageThreshold);


/*!
 *
 */
svkImageThreshold::svkImageThreshold()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    // We have to call this in the constructor.
    this->SetNumberOfInputPorts(6);
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SERIES_DESCRIPTION, "OUTPUT_SERIES_DESCRIPTION", svkAlgorithmPortMapper::SVK_STRING, false);
    this->GetPortMapper()->InitializeInputPort( MASK_OUTPUT_VALUE, "MASK_OUTPUT_VALUE", svkAlgorithmPortMapper::SVK_INT, false);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_MIN, "THRESHOLD_MIN", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( THRESHOLD_MAX, "THRESHOLD_MAX", svkAlgorithmPortMapper::SVK_DOUBLE, false);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_SCALAR_TYPE, "OUTPUT_SCALAR_TYPE", svkAlgorithmPortMapper::SVK_INT, false);
    this->SetNumberOfOutputPorts(1);
    this->GetPortMapper()->InitializeOutputPort( 0, "THRESHOLDED_OUTPUT", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA);
}


/*!
 *
 */
svkImageThreshold::~svkImageThreshold()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~svkImageThreshold()");
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetThresholdMax( double max )
{
    this->GetPortMapper()->SetDoubleInputPortValue( THRESHOLD_MAX, max);
}


/*!
 * Utility getter for input port .
 */
svkDouble* svkImageThreshold::GetThresholdMax( )
{
    return this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_MAX );
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetThresholdMin( double min )
{
    this->GetPortMapper()->SetDoubleInputPortValue( THRESHOLD_MIN, min);
}


/*!
 * Utility getter for input port .
 */
svkDouble* svkImageThreshold::GetThresholdMin( )
{
    return this->GetPortMapper()->GetDoubleInputPortValue( THRESHOLD_MIN );
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetMaskOutputValue( int value )
{
    this->GetPortMapper()->SetIntInputPortValue( MASK_OUTPUT_VALUE, value);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageThreshold::GetMaskOutputValue( )
{
    return this->GetPortMapper()->GetIntInputPortValue( MASK_OUTPUT_VALUE );
}


/*!
 * Utility setter for input port .
 */
void svkImageThreshold::SetMaskSeriesDescription( string description )
{
    this->GetPortMapper()->SetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION, description);
}


/*!
 * Utility getter for input port .
 */
svkString* svkImageThreshold::GetMaskSeriesDescription( )
{
    return this->GetPortMapper()->GetStringInputPortValue( OUTPUT_SERIES_DESCRIPTION );
}


/*!
 * Sets the output scalar type.
 */
void svkImageThreshold::SetOutputScalarType( int outputScalarType )
{
    this->GetPortMapper()->SetIntInputPortValue( OUTPUT_SCALAR_TYPE, outputScalarType);
}


/*!
 * Utility getter for input port .
 */
svkInt* svkImageThreshold::GetOutputScalarType( )
{
    return this->GetPortMapper()->GetIntInputPortValue( OUTPUT_SCALAR_TYPE );
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output. 
 */
int svkImageThreshold::RequestData( vtkInformation* request,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector )
{

    double min = this->GetThresholdMin()->GetValue();
    double max = VTK_DOUBLE_MAX;
    if( this->GetThresholdMax() != NULL  ) {
        max = this->GetThresholdMax()->GetValue();
    }

    string description = this->GetImageDataInput(0)->GetDcmHeader()->GetStringValue("SeriesDescription");
    description.append(" Mask");

    if( this->GetMaskSeriesDescription() != NULL ) {
        description = this->GetMaskSeriesDescription()->GetValue();
    }

    vtkImageThreshold* threshold = vtkImageThreshold::New();
    if( this->GetOutputScalarType() != NULL ) {
        threshold->SetOutputScalarType( this->GetOutputScalarType()->GetValue() );
    }
    threshold->SetOutValue(0);

    // The MASK_OUTPUT_VALUE is optional so check to see if its set
    if(this->GetMaskOutputValue() != NULL ) {
        threshold->SetInValue(this->GetMaskOutputValue()->GetValue());
    }
    threshold->ThresholdBetween( min, max );

    svkImageAlgorithmExecuter* executer = svkImageAlgorithmExecuter::New();
    executer->SetAlgorithm( threshold );

    // Set the input of the vtk algorithm to be the input of the executer
    executer->SetInput(this->GetImageDataInput(0));

    // Update the vtk algorithm
    executer->Update();

    // Copy the output of the vtk algorithm
    this->GetOutput()->DeepCopy( executer->GetOutput() );
    this->GetOutput()->GetDcmHeader()->SetValue("SeriesDescription", description );
    if( this->GetOutputScalarType() != NULL ) {
        this->GetOutput()->GetDcmHeader()->SetPixelDataType( svkDcmHeader::GetVtkDataTypeFromSvkDataType( this->GetOutputScalarType()->GetValue() ));
    }
    threshold->Delete();
    executer->Delete();
    return 1; 
}
