/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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


#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLUtilities.h>
#include <vtkImageMathematics.h>

#include <svkQuantifyMetabolites.h>
#include <svkMetaboliteMap.h>
#include <svkImageCopy.h>
#include <svkMetaboliteRatioZScores.h>

#include <time.h>
#include <sys/stat.h>


using namespace svk;


vtkCxxRevisionMacro(svkQuantifyMetabolites, "$Rev$");
vtkStandardNewMacro(svkQuantifyMetabolites);


/*!
 *
 */
svkQuantifyMetabolites::svkQuantifyMetabolites()
{
#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    vtkDebugMacro(<< this->GetClassName() << "::" << this->GetClassName() << "()");

    this->isVerbose = false; 
    this->xmlFileName = svkQuantifyMetabolites::GetDefaultXMLFileName(); 
    this->mrsXML = NULL;
    this->useSelectedVolumeFraction = 0;
    this->selectedVolumeMask = NULL;

}


/*!
 *
 */
svkQuantifyMetabolites::~svkQuantifyMetabolites()
{
    vtkDebugMacro(<<this->GetClassName()<<"::~"<<this->GetClassName());
}


/* 
 *  Parses the XML quantification parameters and generate set of metabolite maps.  
 */
void svkQuantifyMetabolites::Quantify()
{

    if ( this->mrsXML == NULL ) {
        this->mrsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() ); 
    }

    this->GenerateRegionMaps();
    this->GenerateRatioMaps(); 
    this->GenerateZScoreMaps(); 
    //this->mrsXML->PrintXML(cout, vtkIndent()); 

}


/*
 *  Retruns a pointer to the metabolite map vector
 */
vtkstd::vector< svkMriImageData* >* svkQuantifyMetabolites::GetMetMaps()
{
    return &(this->metMapVector);        
}


/*
 *
 */
void svkQuantifyMetabolites::GenerateRegionMaps()
{

    this->GetRegionNameVector(); 

    //  
    //  Loop over the regions to quantify:
    //  
    bool parseQuants = true; 
    int quantIndex = 0; 
    vtkXMLDataElement* quantXML;
    vtkXMLDataElement* algoXML;
    svkMetaboliteMap* mapGen = svkMetaboliteMap::New();
    mapGen->SetInput( this->GetImageDataInput(0) ); 

    while ( parseQuants ) {
        ostringstream ossIndex;
        ossIndex << quantIndex;
        vtkstd::string quantIndexString(ossIndex.str());

        quantXML = this->mrsXML->FindNestedElementWithNameAndId("QUANT", quantIndexString.c_str());

        if (quantXML != NULL ) {

            //cout << "quant id = " << quantIndexString << endl;
            quantXML->PrintXML( cout, vtkIndent() ); 

            //  Get region and algo to quantify.  peak range is defied already in regionVector;  
            //  image to store in a vtkstd::vector of svkMriImageData objects. 
            int regionID; 
            quantXML->GetScalarAttribute("region", regionID); 
            algoXML = quantXML->FindNestedElementWithName("ALGO");
            vtkstd::string algoName( algoXML->GetAttributeValue( 0 ) );
            algoName = this->ReplaceSlashesAndWhiteSpace( algoName );

            vtkstd::string regionName = this->regionVector[regionID][0];  
            float peakPPM  =  GetFloatFromString( this->regionVector[regionID][1] ); 
            float widthPPM =  GetFloatFromString( this->regionVector[regionID][2] ); 

            //cout << "NAME      : " << regionName << endl;
            //cout << "PEAK POS  : " << peakPPM << endl; 
            //cout << "PEAK WIDTH: " << widthPPM << endl; 
            //cout << "ALGO      : " << algoName << endl;

            mapGen->SetPeakPosPPM( peakPPM );
            mapGen->SetPeakWidthPPM( widthPPM ); 
            mapGen->SetSeriesDescription( regionName + "_" + algoName );
            mapGen->SetAlgorithm( algoName );

            if ( this->useSelectedVolumeFraction ) { 
                mapGen->LimitToSelectedVolume(); 
            }
            mapGen->Update();

            svkMriImageData* image = svkMriImageData::New();
            image->DeepCopy( mapGen->GetOutput() );

            this->metMapVector.push_back( image );     

        } else {

            parseQuants = false; 

        }

        quantIndex++; 
    }

    //  If masking, grab a copy to use for z-score computation
    if ( this->useSelectedVolumeFraction ) { 
        
        int numVoxels = this->metMapVector[0]->GetDcmHeader()->GetIntValue("Columns"); 
        numVoxels    *= this->metMapVector[0]->GetDcmHeader()->GetIntValue("Rows"); 
        numVoxels    *= this->metMapVector[0]->GetDcmHeader()->GetNumberOfSlices(); 
        this->selectedVolumeMask = new short[numVoxels];
        memcpy( this->selectedVolumeMask, mapGen->GetSelectedVolumeMask(), numVoxels * sizeof(short) );    

    }    
    mapGen->Delete();
 
}


/*
 *
 */
int svkQuantifyMetabolites::GetIntFromString(vtkstd::string stringVal ) 
{
 
    istringstream* iss = new istringstream();
    int intVal;
    iss->str( stringVal ); 
    *iss >> intVal;
    delete iss;
    return intVal; 
}


/*
 *
 */
float svkQuantifyMetabolites::GetFloatFromString(vtkstd::string stringVal ) 
{
 
    istringstream* iss = new istringstream();
    float floatVal;
    iss->str( stringVal ); 
    *iss >> floatVal;
    delete iss;
    return floatVal; 
}


/*
 *  Generates the ratio maps for the quantities specified in the input xml 
 *  configuration file. 
 *
 *  If the computation is masked, e.g. by selection box, 
 *  then set division by zero to 
 *  zero, otherwise use actual computed value    
 */
void svkQuantifyMetabolites::GenerateRatioMaps()
{

    //  
    //  Loop over the ratios to quantify:
    //  
    bool parseRatios = true; 
    int ratioIndex = 0; 
    vtkXMLDataElement* ratioXML;

    while ( parseRatios ) {

        ostringstream ossIndex;
        ossIndex << ratioIndex;
        vtkstd::string ratioIndexString(ossIndex.str());

        ratioXML = this->mrsXML->FindNestedElementWithNameAndId("RATIO", ratioIndexString.c_str());

        if (ratioXML != NULL ) {

            //  Get images contributing to numerator and  denominator for this ratio
             
            //cout << "RXML: " << ratioXML->GetAttributeValue( 1 ) << endl;; 
            vtkstd::string ratioName( ratioXML->GetAttributeValue( 1 ) ); 
            ratioName = this->ReplaceSlashesAndWhiteSpace( ratioName ); 
            //cout << "RATIO NAME : " << ratioName << endl;
            
            //  Initialize the ratio with zeros:
            svkImageData* numeratorImage   = svkMriImageData::New();
            svkImageData* denominatorImage = svkMriImageData::New();
            numeratorImage->ZeroCopy( this->metMapVector[0] ); 
            denominatorImage->ZeroCopy( this->metMapVector[0] ); 

            //  Get the numerator and denominator values for this ratio
            this->GetNumeratorAndDenominatorImages( ratioXML, numeratorImage, denominatorImage); 

            vtkImageMathematics* mathR = vtkImageMathematics::New(); 
            mathR->SetOperationToDivide();

            //  If the computation is masked, e.g. by selection box, 
            //  then set division by zero to 
            //  zero, otherwise use actual computed value    
            if ( this->useSelectedVolumeFraction ) { 
                mathR->SetConstantC(0);
                mathR->DivideByZeroToCOn();
            }
            mathR->SetInput1( numeratorImage ); 
            mathR->SetInput2( denominatorImage ); 
            mathR->Update();


            //  Create a new image a copy from a correctly 
            //  instantiated object, then copy over the new pixels
            //  from the vtkImageData object
            svkMriImageData* ratioImage = svkMriImageData::New();
            ratioImage->DeepCopy( this->metMapVector[0] );
            ratioImage->DeepCopy( mathR->GetOutput() );

            //  set the header fields
            vtkstd::string seriesDescription( ratioName );
            svkDcmHeader* hdr = ratioImage->GetDcmHeader();
            hdr->SetValue("SeriesDescription", ratioName );

            this->metMapVector.push_back( ratioImage ); 

        } else {

            parseRatios = false; 

        }

        ratioIndex++; 
    }

}


/*
 *  Sums metabolite imgages contributing to numerator and denominator of image ratios or z-scores.   
 */
void svkQuantifyMetabolites::GetNumeratorAndDenominatorImages( vtkXMLDataElement* ratioXML, svkImageData* numeratorImage, svkImageData* denominatorImage)
{

    vtkImageMathematics* mathN = vtkImageMathematics::New(); 
    vtkImageMathematics* mathD = vtkImageMathematics::New(); 
    mathN->SetOperationToAdd();
    mathD->SetOperationToAdd();
    mathN->SetInput1( numeratorImage );     
    mathD->SetInput1( denominatorImage );     

    svkImageData* inputImage = svkMrsImageData::New();
    inputImage->ZeroCopy( this->metMapVector[0] );

    bool elementsExist = true; 
    int elementID = 0; 
    while ( elementsExist ) {

        vtkXMLDataElement* nestedXML = ratioXML->GetNestedElement(elementID); 
        if ( nestedXML != NULL ) {

            //  Is it part of numerator or denominator?
            vtkstd::string quantType (nestedXML->GetName() ); 
            vtkstd::string quantNumString = nestedXML->GetAttributeValue(0);
            istringstream* iss = new istringstream();
            int quantNum;
            iss->str( quantNumString ); 
            *iss >> quantNum;
            delete iss;
            elementID++;    
            if ( quantType.compare("NUMERATOR") == 0 ) {
                inputImage->DeepCopy( this->metMapVector[quantNum] );
                mathN->SetInput2( inputImage ); 
                mathN->Update();
                numeratorImage->DeepCopy( mathN->GetOutput() ); 
                mathN->SetInput1( numeratorImage ); 
            } else if ( quantType.compare("DENOMINATOR") == 0 ) {
                mathD->SetInput2( this->metMapVector[quantNum] ); 
                mathD->Update();
                denominatorImage->DeepCopy( mathD->GetOutput() ); 
                mathD->SetInput1( denominatorImage ); 
            }
            
        } else {

            elementsExist = false; 
        }
    }

    mathN->Delete();
    mathD->Delete();
}


/*
 *  Generates z-score maps (metabolite indices) for the quantities specified in 
 *  the input xml configuration file.  
 */
void svkQuantifyMetabolites::GenerateZScoreMaps()
{

    //  
    //  Loop over the zscores to quantify (while loop):
    //  
    bool parseZScores= true; 
    int zscoreIndex = 0; 
    vtkXMLDataElement* zscoreXML;

    svkMetaboliteRatioZScores* zscore = svkMetaboliteRatioZScores::New(); 

    while ( parseZScores) {

        ostringstream ossIndex;
        ossIndex << zscoreIndex;
        vtkstd::string zscoreIndexString(ossIndex.str());

        zscoreXML = this->mrsXML->FindNestedElementWithNameAndId("ZSCORE", zscoreIndexString.c_str());

        if (zscoreXML != NULL ) {

            //  Get images contributing to numerator and  denominator for this index 
             
            //cout << "ZSXML: " << zscoreXML->GetAttributeValue( 1 ) << endl;; 
            vtkstd::string zscoreName( zscoreXML->GetAttributeValue( 1 ) ); 
            zscoreName = this->ReplaceSlashesAndWhiteSpace( zscoreName ); 
            //cout << "ZSCORE NAME : " << zscoreName << endl;
            
            //  Initialize the z-score maps with zeros:
            svkImageData* numeratorImage   = svkMriImageData::New();
            svkImageData* denominatorImage = svkMriImageData::New();
            numeratorImage->ZeroCopy( this->metMapVector[0] ); 
            denominatorImage->ZeroCopy( this->metMapVector[0] ); 

            //  Get the numerator and denominator images for 
            //  this ratio:
            this->GetNumeratorAndDenominatorImages( zscoreXML, numeratorImage, denominatorImage); 

            //  Now generate the z-score form the 
            //  numerator/denominator images
            zscore->SetInputNumerator( numeratorImage ); 
            zscore->SetInputDenominator( denominatorImage ); 
            zscore->SetInputMrsData( this->GetImageDataInput(0) ); 
            if ( this->useSelectedVolumeFraction ) { 
                zscore->LimitToSelectedVolume( this->selectedVolumeMask ); 
            }
            zscore->Update();

            //  Create a new image a copy from a correctly 
            //  instantiated object, then copy over the new pixels
            //  from the vtkImageData object
            svkMriImageData* zscoreImage = svkMriImageData::New();
            zscoreImage->DeepCopy( zscore->GetOutput() );

            //  set the header fields
            vtkstd::string seriesDescription( zscoreName );
            svkDcmHeader* hdr = zscoreImage->GetDcmHeader();
            hdr->SetValue("SeriesDescription", zscoreName );

            this->metMapVector.push_back( zscoreImage ); 

        } else {

            parseZScores = false; 

        }

        zscoreIndex++; 
    }

    zscore->Delete(); 

}


/*
 *  Limits the calculation to voxels that have at least the specified fractional
 *  volume within the selected MRS volume. The default is to include all voxels
 *  in the calculation (fraction = 0).
 */
void svkQuantifyMetabolites::LimitToSelectedVolume(float fraction)
{
    this->useSelectedVolumeFraction = fraction;
    this->Modified();
}


/*!
 *  Resets the origin and extent for correct initialization of output svkMriImageData object from input 
 *  svkMrsImageData object. 
 */
int svkQuantifyMetabolites::RequestInformation( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{
    return 1;
}


/*!
 *  Copy the Dcm Header and Provenance from the input to the output. 
 */
int svkQuantifyMetabolites::RequestData( vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector )
{

    this->metMapVector.clear();
    this->Quantify();

    return 1; 
};


/*
 *  Replace ('/') slash character and white space in name attributes with "_div_" 
 *  and "_" respectively so we end up with valid file names. 
 */
string svkQuantifyMetabolites::ReplaceSlashesAndWhiteSpace( vtkstd::string inString)
{
   
    vtkstd::string outString( inString ) ;

    size_t slashPos;
    slashPos = outString.find_first_of("/");

    while ( slashPos != vtkstd::string::npos ) {
        outString.assign( outString.replace(slashPos, 1, "_div_") );
        slashPos = outString.find_first_of("/");
    }

    
    size_t whitePos;
    whitePos = outString.find_first_of(" ");

    while ( whitePos != vtkstd::string::npos ) {
        outString.assign( outString.replace(whitePos, 1, "_") );
        whitePos = outString.find_first_of(" ");
    }

    return outString; 

}


/*!
 *  Returns an stl map containing the region name, with a vector of 2 floats 
 *  that store the region peak ppm and width in ppm.  All info parsed from the 
 *  xml config file.
 */
vtkstd::vector< vtkstd::vector< vtkstd::string > >  svkQuantifyMetabolites::GetRegionNameVector()
{

    this->regionVector.clear();

    //
    //  Loop over the regions to quantify:
    //
    bool parseRegions = true;
    int regionIndex = 0;

    if ( this->mrsXML == NULL ) {
        this->mrsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
    }

    vtkXMLDataElement* regionXML;


    while ( parseRegions ) {

        ostringstream ossIndex;
        ossIndex << regionIndex;
        vtkstd::string regionIndexString(ossIndex.str());

        regionXML = this->mrsXML->FindNestedElementWithNameAndId("REGION", regionIndexString.c_str());

        if (regionXML != NULL ) {

            vtkstd::string regionName( regionXML->GetAttributeValue( 1 ) );
            regionName = this->ReplaceSlashesAndWhiteSpace( regionName );
            string peakPPM( regionXML->GetAttribute("peak_ppm") );
            string widthPPM( regionXML->GetAttribute("width_ppm") );

            //cout << "XX peak pos  : " << peakPPM << endl;
            //cout << "XX peak width: " << widthPPM << endl;

            vtkstd::vector < vtkstd::string > peak;
            peak.push_back(regionName); 
            peak.push_back(peakPPM); 
            peak.push_back(widthPPM); 

            this->regionVector.push_back(peak);  

        } else {
            parseRegions = false; 

        }
        regionIndex++; 
    }

    return this->regionVector; 
}


/*!
 *  Modify a region's peak and width. 
 */
void svkQuantifyMetabolites::ModifyRegion( int regionID, float peakPPM, float widthPPM )
{

    //  If necessary, read xml from file before modifying content:
    if ( this->mrsXML == NULL ) {
        this->mrsXML = vtkXMLUtilities::ReadElementFromFile( this->xmlFileName.c_str() );
    }

    vtkXMLDataElement* regionXML;

    ostringstream ossIndex;
    ossIndex << regionID;
    vtkstd::string regionIndexString(ossIndex.str());

    regionXML = this->mrsXML->FindNestedElementWithNameAndId("REGION", regionIndexString.c_str());

    if (regionXML != NULL ) {

        //  Modify regionXML entry: 
        regionXML->SetFloatAttribute("peak_ppm", peakPPM);
        regionXML->SetFloatAttribute("width_ppm", widthPPM);

    } else {
        vtkWarningWithObjectMacro(this, "Could not find region " + regionIndexString + " in XML config");
    }

    //regionXML->PrintXML(cout, vtkIndent()); 
    //this->mrsXML->PrintXML(cout, vtkIndent()); 

}


/*!
 *  set the path/name to xml file.   
 */
void svkQuantifyMetabolites::SetXMLFileName( string xmlFileName )
{
    this->xmlFileName = xmlFileName;  
}


/*!
 *
 */
void svkQuantifyMetabolites::UpdateProvenance()
{
    vtkDebugMacro(<<this->GetClassName()<<"::UpdateProvenance()");
}


/*!
 *  Write the integrals for each voxel to stdout. Default is false.  
 */
void svkQuantifyMetabolites::SetVerbose( bool isVerbose )
{
    this->isVerbose = isVerbose; 
}


/*!
 *
 */
int svkQuantifyMetabolites::FillInputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "svkMrsImageData");
    return 1;
}


/*!
 *  Output from this algo is an svkMriImageData object. 
 */
int svkQuantifyMetabolites::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info )
{
    info->Set( vtkDataObject::DATA_TYPE_NAME(), "svkMriImageData"); 
    return 1;
}


/*!
 *  gets the default xml quantification template filename 
 *  $HOME/.SIVICQuant.xml
 */
string svkQuantifyMetabolites::GetDefaultXMLFileName()
{
    string fileName = getenv("HOME"); 
    fileName.append("/.SIVICQuant.xml" );
    return fileName;
}


/*!
 *  Print out a template xml config file:  
 *  if fileName is "", then use default filename ( $HOME/.SIVICQuant.xml)
 *  if clober is false, will not overwrite existing file.
 */
void svkQuantifyMetabolites::WriteDefaultXMLTemplate( string fileName, bool clobber )
{

    //  if no filename, use default
    if ( fileName.size() == 0 ) { 
        fileName =  svkQuantifyMetabolites::GetDefaultXMLFileName(); 
    }

    //  first check if the file exists (returns 0):
    struct stat buf;
    bool fileExists; 
    if (stat(fileName.c_str(), &buf) == 0) {
        fileExists = true;
    } else {
        fileExists = false;
    }

    //  if the file doesn't exist, or clobber is true then create it
    if ( ( fileExists && clobber == true ) || fileExists == false ) {

        ofstream xmlOut( fileName.c_str() );
        if( !xmlOut ) {
            throw runtime_error("Cannot open xml file for writing: " + fileName );
        }

        xmlOut << ""
        << "<!-- " << endl
        << "   Copyright © 2009-2011 The Regents of the University of California." << endl
        << "   All Rights Reserved." << endl
        << " " << endl
        << "   Redistribution and use in source and binary forms, with or without" << endl
        << "   modification, are permitted provided that the following conditions are met:" << endl
        << "   •   Redistributions of source code must retain the above copyright notice," << endl
        << "       this list of conditions and the following disclaimer." << endl
        << "   •   Redistributions in binary form must reproduce the above copyright notice," << endl
        << "       this list of conditions and the following disclaimer in the documentation" << endl
        << "       and/or other materials provided with the distribution." << endl
        << "   •   None of the names of any campus of the University of California, the name" << endl
        << "       \"The Regents of the University of California,\" or the names of any of its" << endl
        << "       contributors may be used to endorse or promote products derived from this" << endl
        << "       software without specific prior written permission." << endl
        << " " << endl
        << "   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND" << endl
        << "   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" << endl
        << "   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED." << endl
        << "   IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT," << endl
        << "   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT" << endl
        << "   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR" << endl
        << "   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY," << endl
        << "   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)" << endl
        << "   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY" << endl
        << "   OF SUCH DAMAGE." << endl
        << " " << endl
        << " " << endl
        << "  $URL$" << endl
        << "  $Rev$" << endl
        << "   $Author$" << endl
        << "   $Date$" << endl
        << " " << endl
        << "   Authors:" << endl
        << "       Jason C. Crane, Ph.D." << endl
        << "       Beck Olson" << endl
        << "-->" << endl
        << " "  << endl
        << " <SVK_MRS_QUANTIFICATION version=\"0.7.6\">" << endl
        << " " << endl
        << "     <REGION id=\"0\" name=\"CHOLINE\"   peak_ppm=\"3.1455\" width_ppm=\".1758\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"1\" name=\"CREATINE\"  peak_ppm=\"3.06\"  width_ppm=\".18\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"2\" name=\"NAA\"       peak_ppm=\"1.9833\"  width_ppm=\".13\">" << endl
        << "     </REGION>" << endl
        << "     <REGION id=\"3\" name=\"LIPID_LAC\" peak_ppm=\"1.36\"  width_ppm=\".17\">" << endl
        << "     </REGION>" << endl
        << " " << endl
        << "     -- cho peak ht" << endl
        << "     <QUANT id=\"0\" region=\"0\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- creatine peak ht" << endl
        << "     <QUANT id=\"1\" region=\"1\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- naa peak ht" << endl
        << "     <QUANT id=\"2\" region=\"2\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- lip/lac peak ht " << endl
        << "     <QUANT id=\"3\" region=\"3\">" << endl
        << "         <ALGO name=\"PEAK_HT\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- cho integrated area" << endl
        << "     <QUANT id=\"4\" region=\"0\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- creatine integrated area" << endl
        << "     <QUANT id=\"5\" region=\"1\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- NAAintegrated area" << endl
        << "     <QUANT id=\"6\" region=\"2\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- lip/lac integrated area " << endl
        << "     <QUANT id=\"7\" region=\"3\">" << endl
        << "         <ALGO name=\"INTEGRATE\">" << endl
        << "         </ALGO>" << endl
        << "     </QUANT>" << endl
        << " " << endl
        << "     -- ratio of choline to naa peak ht" << endl
        << "     <RATIO id=\"0\" name=\"CHO/NAA_PEAK_HT\">" << endl
        << "         <NUMERATOR quant_id=\"0\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"2\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </RATIO>" << endl
        << " " << endl
        << "     -- ratio of choline + creatine to naa peak ht" << endl
        << "     <RATIO id=\"2\" name=\"CHO+CRE/NAA INTEGRATE\">" << endl
        << "         <NUMERATOR quant_id=\"4\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <NUMERATOR quant_id=\"5\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"6\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </RATIO>" << endl
        << " " << endl
        << "     -- choline/naa peak ht index (z-score)" << endl
        << "     <ZSCORE id=\"0\" name=\"CNI PEAK HT\">" << endl
        << "         <NUMERATOR quant_id=\"0\">" << endl
        << "         </NUMERATOR>" << endl
        << "         <DENOMINATOR quant_id=\"2\">" << endl
        << "         </DENOMINATOR>" << endl
        << "     </ZSCORE>" << endl
        << " " << endl
        << " " << endl
        << " </SVK_MRS_QUANTIFICATION> " << endl; 

            xmlOut.close();
    }

}