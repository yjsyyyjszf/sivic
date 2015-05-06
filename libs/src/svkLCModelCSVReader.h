/*
 *  Copyright © 2009-2015 The Regents of the University of California.
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


#ifndef SVK_LCMODEL_CSV_READER_H
#define SVK_LCMODEL_CSV_READER_H

#include <vtkInformation.h>
#include <vtkstd/map>
#include <vtkStringArray.h>
#include <vtkstd/string>

#include <svkUtils.h>
#include <svkImageReader2.h>
#include <svkEnhancedMRIIOD.h>


namespace svk {


/*! 
 *  
 */
class svkLCModelCSVReader : public svkImageReader2 
{

    public:

        static svkLCModelCSVReader* New();
        vtkTypeRevisionMacro( svkLCModelCSVReader, svkImageReader2);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "LCModel CSV File";
        }

        //  Methods:
        virtual int             CanReadFile( const char* fname );
        void                    SetMetName( string metName );
        void                    SetMRSFileName( string mrsFileName );

    protected:

        svkLCModelCSVReader();
        ~svkLCModelCSVReader();

        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual int                              FillInputPortInformation( int port, vtkInformation* info ); 
        virtual void                             ExecuteInformation();
        virtual void                             ExecuteData(vtkDataObject *output);
        virtual svkDcmHeader::DcmPixelDataFormat GetFileType();


    private:

        //  Methods:
        void            ParseCSVFiles();
        virtual void    InitDcmHeader();
        string          GetSeriesDescription();
        void            GetVoxelIndexFromFileName(string csvFileName, int* col, int* row, int* slice); 



        //  Members:
        void*                                   pixelData; 
        vtkStringArray*                         tmpFileNames;
        vtkDoubleArray*                         csvPixelValues; 
        vtkIntArray*                            csvColIndex; 
        vtkIntArray*                            csvRowIndex; 
        string                                  metName;
        string                                  mrsFileName;

};


}   //svk


#endif //SVK_LCMODEL_CSV_READER_H
