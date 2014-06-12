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
 *
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


#include <svkImageStatistics.h>

using namespace svk;

vtkCxxRevisionMacro(svkImageStatistics, "$Rev$");
vtkStandardNewMacro(svkImageStatistics);

//! Constructor
svkImageStatistics::svkImageStatistics()
{
    this->SetNumberOfInputPorts(24);
    this->SetNumberOfOutputPorts(1);
    bool required = true;
    bool repeatable = true;
    this->GetPortMapper()->InitializeInputPort( INPUT_IMAGE, "INPUT_IMAGE", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( INPUT_ROI, "INPUT_ROI", svkAlgorithmPortMapper::SVK_MR_IMAGE_DATA, required, repeatable );
    this->GetPortMapper()->InitializeInputPort( NUM_BINS, "NUM_BINS", svkAlgorithmPortMapper::SVK_INT);
    this->GetPortMapper()->InitializeInputPort( BIN_SIZE, "BIN_SIZE", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( START_BIN, "START_BIN", svkAlgorithmPortMapper::SVK_DOUBLE);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_HISTOGRAM, "COMPUTE_HISTOGRAM", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEAN, "COMPUTE_MEAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MAX, "COMPUTE_MAX", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MIN, "COMPUTE_MIN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_STDEV, "COMPUTE_STDEV", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VOLUME, "COMPUTE_VOLUME", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MODE, "COMPUTE_MODE", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_QUANTILES, "COMPUTE_QUANTILES", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MEDIAN, "COMPUTE_MEDIAN", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SUM, "COMPUTE_SUM", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MOMENT_2, "COMPUTE_MOMENT_2", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MOMENT_3, "COMPUTE_MOMENT_3", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_MOMENT_4, "COMPUTE_MOMENT_4", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_VARIANCE, "COMPUTE_VARIANCE", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SAMPLE_KURTOSIS, "COMPUTE_SAMPLE_KURTOSIS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_SAMPLE_SKEWNESS, "COMPUTE_SAMPLE_SKEWNESS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_POPULATION_KURTOSIS, "COMPUTE_POPULATION_KURTOSIS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( COMPUTE_POPULATION_SKEWNESS, "COMPUTE_POPULATION_SKEWNESS", svkAlgorithmPortMapper::SVK_BOOL, !required);
    this->GetPortMapper()->InitializeInputPort( OUTPUT_FILE_NAME, "OUTPUT_FILE_NAME", svkAlgorithmPortMapper::SVK_STRING, !required);
    this->GetPortMapper()->InitializeOutputPort( 0, "XML_RESULTS", svkAlgorithmPortMapper::SVK_XML);
}


//! Destructor
svkImageStatistics::~svkImageStatistics()
{
}


/*
*/
vtkXMLDataElement* svkImageStatistics::GetOutput()
{
   vtkXMLDataElement* output = NULL;
   svkXML* outputDataObject = svkXML::SafeDownCast(this->GetOutputDataObject(0));
   if( outputDataObject != NULL ) {
       output = outputDataObject->GetValue();
   }
   return output;
}


/*!
 *  RequestData pass the input through the algorithm, and copies the dcos and header
 *  to the output.
 */
int svkImageStatistics::RequestData( vtkInformation* request,
                                              vtkInformationVector** inputVector,
                                              vtkInformationVector* outputVector )
{
    vtkXMLDataElement* results = this->GetOutput();

    results->RemoveAllNestedElements();
    results->RemoveAllAttributes();
    results->SetName("svk_image_stats_results");

    results->SetAttribute("version", string(SVK_RELEASE_VERSION).c_str());
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    string timeString = asctime(timeinfo);
    // Chop off the return character from asctime:
    timeString = timeString.substr(0, timeString.size()-1);
    results->SetAttribute("date", timeString.c_str());

    for (int imageIndex = 0; imageIndex < this->GetNumberOfInputConnections(INPUT_IMAGE); imageIndex++) {
        for (int roiIndex = 0; roiIndex < this->GetNumberOfInputConnections(INPUT_ROI); roiIndex++) {
            svkMriImageData* image = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_IMAGE, imageIndex);
            svkMriImageData* roi   = this->GetPortMapper()->GetMRImageInputPortValue(INPUT_ROI, roiIndex);
            vtkXMLDataElement* nextResult = vtkXMLDataElement::New();
            nextResult->SetName("results");
            string imageLabel = image->GetDcmHeader()->GetStringValue("SeriesDescription");
            string roiLabel = roi->GetDcmHeader()->GetStringValue("SeriesDescription");
            svkUtils::CreateNestedXMLDataElement( nextResult, "IMAGE", imageLabel);
            svkUtils::CreateNestedXMLDataElement( nextResult, "ROI",   roiLabel);
            vtkXMLDataElement* statistics = vtkXMLDataElement::New();
            bool geometriesMatch = true;
            if( roi != NULL ) {
                svkDataValidator* validator = svkDataValidator::New();
                if( !validator->AreDataGeometriesSame( image, roi ) ) {
                    cout << "ERROR: Data geometries do not match between image and ROI. Cannot compute order statistics:" << validator->resultInfo << endl;
                    geometriesMatch = false;
                }
                validator->Delete();
            }
            statistics->SetName("measures");
            if( geometriesMatch ) {
                this->ComputeOrderStatistics(image,roi, statistics);
                this->ComputeAccumulateStatistics(image,roi, statistics);
                this->ComputeDescriptiveStatistics(image,roi, statistics);
            }
            vtkIndent indent;
            if( statistics != NULL ) {
                nextResult->AddNestedElement( statistics );
            }
            results->AddNestedElement( nextResult );
            nextResult->Delete();
        }
    }

    vtkIndent indent;
    svkString* outputFileName = this->GetPortMapper()->GetStringInputPortValue(OUTPUT_FILE_NAME);
    if( outputFileName != NULL && outputFileName->GetValue() != "") {
        vtkXMLUtilities::WriteElementToFile( results, outputFileName->GetValue().c_str());
    }
    return 1;
}


/*!
 * Computes basic statistics using vtkImageAccumulate.
 */
void svkImageStatistics::ComputeAccumulateStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        double* spacing = image->GetSpacing();
        double pixelVolume = spacing[0] * spacing[1] * spacing[2];
        vtkImageAccumulate* accumulator = vtkImageAccumulate::New();
        accumulator->SetInput( image );
        if( roi != NULL ) {
            vtkImageToImageStencil* stencil = vtkImageToImageStencil::New();
            stencil->SetInput( roi );
            stencil->ThresholdByUpper(1);
            stencil->Update();
            accumulator->SetStencil( stencil->GetOutput() );
            stencil->Delete();
        }
        accumulator->Update( );
        accumulator->SetIgnoreZero( false );
        int numberOfBins = this->GetPortMapper()->GetIntInputPortValue( NUM_BINS )->GetValue();
        double startBin  = this->GetPortMapper()->GetDoubleInputPortValue( START_BIN )->GetValue();
        double binSize   = this->GetPortMapper()->GetDoubleInputPortValue( BIN_SIZE )->GetValue();
        accumulator->SetComponentExtent(0,numberOfBins-1,0,0,0,0 );
        accumulator->SetComponentOrigin(startBin, 0,0 );
        accumulator->SetComponentSpacing(binSize, 0,0);
        accumulator->Update();
        if( this->GetShouldCompute(COMPUTE_VOLUME)) {
            string volumeString = svkUtils::DoubleToString( accumulator->GetVoxelCount()*pixelVolume );
            svkUtils::CreateNestedXMLDataElement( results, "volume", volumeString);
        }

        if( this->GetShouldCompute(COMPUTE_MAX)) {
            string maxString = svkUtils::DoubleToString( *accumulator->GetMax() );
            svkUtils::CreateNestedXMLDataElement( results, "max", maxString);
        }


        if( this->GetShouldCompute(COMPUTE_MIN)) {
            string minString = svkUtils::DoubleToString( *accumulator->GetMin() );
            svkUtils::CreateNestedXMLDataElement( results, "min", minString);
        }

        if( this->GetShouldCompute(COMPUTE_MEAN)) {
            string meanString = svkUtils::DoubleToString( *accumulator->GetMean() );
            svkUtils::CreateNestedXMLDataElement( results, "mean", meanString);
        }

        if( this->GetShouldCompute(COMPUTE_STDEV)) {
            string stdevString = svkUtils::DoubleToString( *accumulator->GetStandardDeviation() );
            svkUtils::CreateNestedXMLDataElement( results, "stdev", stdevString);
        }

        if( this->GetShouldCompute(COMPUTE_MODE)) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            double max = *accumulator->GetMax();
            double min = *accumulator->GetMin();
            int numBins =  histData->GetNumberOfTuples();
            accumulator->Update();
            if( numBins > 0 && histData != NULL ) {
                double mode = 0;
                double binMax = 0;
                for( int i = 0; i < numBins; i++ ) {
                    if( histData->GetTuple1(i) > binMax ) {
                        binMax = histData->GetTuple1(i);
                        mode = startBin + (i+0.5)*binSize;
                    }
                }
                string valueString = svkUtils::DoubleToString(mode);
                svkUtils::CreateNestedXMLDataElement( results, "mode", valueString);
            }
        }

        if( this->GetShouldCompute(COMPUTE_HISTOGRAM)) {
            vtkDataArray* histData = accumulator->GetOutput()->GetPointData()->GetScalars();
            double max = *accumulator->GetMax();
            double min = *accumulator->GetMin();
            int numBins =  histData->GetNumberOfTuples();
            accumulator->Update();
            vtkXMLDataElement* histogram = vtkXMLDataElement::New();
            histogram->SetName("histogram");
            histogram->SetAttribute("bins", svkUtils::IntToString(numBins).c_str());
            for( int i = 0; i < numBins; i++ ) {
                vtkXMLDataElement* element = vtkXMLDataElement::New();
                element->SetName("bin");
                element->SetAttribute("index", svkUtils::IntToString(i).c_str());
                element->SetAttribute("min", svkUtils::DoubleToString(startBin + i*binSize).c_str());
                element->SetAttribute("max", svkUtils::DoubleToString(startBin + (i+1)*binSize).c_str());
                string valueString = svkUtils::DoubleToString(histData->GetTuple1(i));
                element->SetCharacterData(valueString.c_str(), valueString.size());
                histogram->AddNestedElement(element);
                element->Delete();
            }
            results->AddNestedElement(histogram);
            histogram->Delete();
        }
        accumulator->Delete();
    }
}


/*!
 * Computes statistics using the vtkOrderStatistics class.
 */
void svkImageStatistics::ComputeOrderStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        vtkDataArray* pixels = image->GetPointData()->GetScalars();
        vtkDataArray* mask   = NULL;
        if( roi != NULL ) {
            mask  = roi->GetPointData()->GetScalars();
        }
        vtkDataArray* pixelsInROI = vtkDataArray::CreateDataArray( pixels->GetDataType());
        pixelsInROI->SetName("PixelsInROI");
        pixelsInROI->SetNumberOfComponents(1);

        for( int i = 0; i < pixels->GetNumberOfTuples(); i++ ) {
            if( mask == NULL || mask->GetTuple1(i) > 0 ) {
                pixelsInROI->InsertNextTuple1( pixels->GetTuple1(i));
            }
        }
        vtkTable* table = vtkTable::New();
        table->AddColumn( pixelsInROI );


        // Compute Quartiles
        vtkOrderStatistics* quartileStatsCalculator = vtkOrderStatistics::New();
        quartileStatsCalculator->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        quartileStatsCalculator->AddColumn(pixelsInROI->GetName());
        quartileStatsCalculator->SetLearnOption(true);
        quartileStatsCalculator->SetAssessOption(false);
        quartileStatsCalculator->Update();
        vtkTable* quartileResults = quartileStatsCalculator->GetOutput(1);

        /*
         // This is usefull for debuging. It prints all the results of the algroithm out.
        for( int i = 0; i < quartileResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < quartileResults->GetNumberOfColumns(); j++) {
                cout << quartileResults->GetColumnName(j) << ": " << quartileResults->GetValue(i,j) << endl;
            }
        }
        */

        // Compute deciles
        vtkOrderStatistics* decilestatsCalculator = vtkOrderStatistics::New();
        decilestatsCalculator->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        decilestatsCalculator->AddColumn(pixelsInROI->GetName());
        decilestatsCalculator->SetLearnOption(true);
        decilestatsCalculator->SetAssessOption(false);
        decilestatsCalculator->SetNumberOfIntervals(10);
        decilestatsCalculator->Update();
        vtkTable* decileResults = decilestatsCalculator->GetOutput(1);

        /*
         // This is usefull for debuging. It prints all the results of the algroithm out.
        for( int i = 0; i < decileResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < decileResults->GetNumberOfColumns(); j++) {
                cout << decileResults->GetColumnName(j) << ": " << decileResults->GetValue(i,j) << endl;
            }
        }
        */

        if( this->GetShouldCompute(COMPUTE_QUANTILES)) {
            string valueString = svkUtils::DoubleToString( decileResults->GetValueByName(0,"0.1-quantile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "tenthPercentile", valueString);
            valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"First Quartile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "firstquartile", valueString);
            valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"Median").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "median", valueString);
            valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"Third Quartile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "thirdquartile", valueString);
            valueString = svkUtils::DoubleToString( decileResults->GetValueByName(0,"0.9-quantile").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "ninetiethPercentile", valueString);
        } else if( this->GetShouldCompute(COMPUTE_MEDIAN)) {
            string valueString = svkUtils::DoubleToString( quartileResults->GetValueByName(0,"Median").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "median", valueString);
        }

        decilestatsCalculator->Delete();
        quartileStatsCalculator->Delete();
        table->Delete();
        pixelsInROI->Delete();
    }
}


/*!
 * Computes statistics using the vtkDescriptiveStatistics class.
 */
void svkImageStatistics::ComputeDescriptiveStatistics(svkMriImageData* image, svkMriImageData* roi, vtkXMLDataElement* results)
{
    if( image != NULL ) {
        vtkDataArray* pixels = image->GetPointData()->GetScalars();
        vtkDataArray* mask   = NULL;
        if( roi != NULL ) {
            mask  = roi->GetPointData()->GetScalars();
        }
        vtkDataArray* pixelsInROI = vtkDataArray::CreateDataArray( pixels->GetDataType());
        pixelsInROI->SetName("PixelsInROI");
        pixelsInROI->SetNumberOfComponents(1);

        for( int i = 0; i < pixels->GetNumberOfTuples(); i++ ) {
            if( mask == NULL || mask->GetTuple1(i) > 0 ) {
                pixelsInROI->InsertNextTuple1( pixels->GetTuple1(i));
            }
        }
        vtkTable* table = vtkTable::New();
        table->AddColumn( pixelsInROI );
        vtkDescriptiveStatistics* descriptiveStats = vtkDescriptiveStatistics::New();
        descriptiveStats->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, table );
        descriptiveStats->AddColumn(pixelsInROI->GetName());
        descriptiveStats->SetLearnOption(true);
        descriptiveStats->SetAssessOption(false);
        descriptiveStats->Update();
        vtkTable* statResults = descriptiveStats->GetOutput(1);
        /*
         // This is usefull for debuging. It prints all the results of the algroithm out.
        cout << "statResults: " << *statResults << endl;
        for( int i = 0; i < statResults->GetNumberOfRows(); i++) {
            for( int j = 0; j < statResults->GetNumberOfColumns(); j++) {
                cout << statResults->GetColumnName(j) << ": " << statResults->GetValue(i,j) << endl;
            }
        }
        */
        if( this->GetShouldCompute(COMPUTE_SUM)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Sum").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "sum", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_2)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"M2").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "moment2", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_3)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"M3").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "moment3", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_MOMENT_4)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"M4").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "moment4", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_VARIANCE)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"Variance").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "variance", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_SAMPLE_KURTOSIS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"g2 Kurtosis").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "samplekurtosis", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_SAMPLE_SKEWNESS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"g1 Skewness").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "sampleskewness", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_POPULATION_KURTOSIS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"G2 Kurtosis").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "populationkurtosis", valueString);
        }
        if( this->GetShouldCompute(COMPUTE_POPULATION_SKEWNESS)){
            string valueString = svkUtils::DoubleToString( statResults->GetValueByName(0,"G1 Skewness").ToDouble() );
            svkUtils::CreateNestedXMLDataElement( results, "populationskewness", valueString);
        }
    }
}


/*!
 * Utility method to check to see if the option has been set to compute a given metric.
 */
bool svkImageStatistics::GetShouldCompute( svkImageStatistics::svkImageStatisticsParameter parameter)
{
    return (this->GetPortMapper()->GetBoolInputPortValue(parameter) && this->GetPortMapper()->GetBoolInputPortValue(parameter)->GetValue());
}
