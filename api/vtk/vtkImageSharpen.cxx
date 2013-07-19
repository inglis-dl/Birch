/*=========================================================================

  Program:   
  Module:    vtkImageSharpen.cxx
  Language:  C++

  Copyright (c) Dean Inglis
  All rights reserved.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.

=========================================================================*/
#include "vtkImageSharpen.h"

#include "vtkImageCast.h"
#include "vtkImageData.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkImageMathematics.h"
#include "vtkImageShiftScale.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <map>

vtkStandardNewMacro(vtkImageSharpen);

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageSharpen::vtkImageSharpen()
{
  this->Radius = 1.;
  this->StandardDeviation = 2.;
  this->Weight = 20.;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkImageSharpen::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the data object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *input = vtkImageData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int inExt[6];
  input->GetExtent(inExt);
  // if the input extent is empty then exit
  if (inExt[1] < inExt[0] ||
      inExt[3] < inExt[2] ||
      inExt[5] < inExt[4])
    {
    return 1;
    }

  int nc = input->GetNumberOfScalarComponents();
  if( nc != 1 )
    {
      vtkErrorMacro("Single component image input required");
      return 1;
    } 

  // Set the extent of the output and allocate memory.
  output->SetExtent(
    outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
  int type = output->GetScalarType();
  if( type == VTK_FLOAT || type == VTK_DOUBLE )
    {
      vtkErrorMacro("Float or double scalar type not supported");
      return 1;  
    }
  output->SetNumberOfScalarComponents( 1 );
  output->SetScalarType( type );
  output->AllocateScalars();

  this->SimpleExecute(input, output);

  return 1;
}

// The switch statement in Execute will call this method with
// the appropriate input type (IT). Note that this assumes
// that the output data type is the same as the input data type.
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
template <class IT>
void vtkImageSharpenExecute(vtkImageSharpen* self, 
                            vtkImageData* input,
                            vtkImageData* output,
                            IT* inPtr,
                            IT* outPtr,
                            double stddev,
                            double radius,
                            double weight)
{
   weight = weight < 0. ? fabs( weight ) : weight;
   if( weight == 0. )
   {
     output->DeepCopy( input );
     return;
   }
 
   double inputRange[2];
   input->GetScalarRange(inputRange);
   int inputMin = static_cast<int>(inputRange[0]);
   int inputMax = static_cast<int>(inputRange[1]);

   double progressGoal = 14.;
   int    progressCount = 0;
   
   // cast the input to float
   //
   vtkNew<vtkImageCast> caster;
   caster->SetOutputScalarTypeToFloat();
   caster->SetInput(input);

   progressCount++;

   stddev = stddev < 0. ? fabs(stddev) : stddev;
   radius = radius < 0. ? fabs(radius) : radius;

   // smooth the input with a gaussian kernel
   //
   vtkNew<vtkImageGaussianSmooth> smoother;
   smoother->SetDimensionality( 2 );
   smoother->SetRadiusFactor( radius );
   smoother->SetStandardDeviation( stddev );
   smoother->SetInputConnection( caster->GetOutputPort() );

   progressCount++;

   // smooth the smoothed input with a gaussian kernel
   //
   vtkNew<vtkImageGaussianSmooth> smoother2;
   smoother2->SetDimensionality( 2 );
   smoother2->SetRadiusFactor( radius );
   smoother2->SetStandardDeviation( stddev );
   smoother2->SetInputConnection( smoother->GetOutputPort() );
   smoother2->Update();

   self->UpdateProgress(progressCount++/progressGoal);

   vtkNew<vtkImageData> smooth1;
   smooth1->DeepCopy( smoother->GetOutput() );

   self->UpdateProgress(progressCount++/progressGoal);

   // compute the difference of gaussians
   //
   vtkNew<vtkImageMathematics> m1;
   m1->SetOperationToSubtract();
   m1->SetInput1( smooth1.GetPointer() );
   m1->SetInput2( smoother2->GetOutput() );
   m1->Update();

   self->UpdateProgress(progressCount++/progressGoal);

   // add the scaled difference of gaussians to the input
   //
   vtkNew<vtkImageMathematics> m2;
   m2->SetOperationToMultiplyByK();
   m2->SetConstantK( weight );
   m2->SetInput1( m1->GetOutput() );
   m2->Update();

   self->UpdateProgress(progressCount++/progressGoal);

   vtkNew<vtkImageData> orig;
   orig->DeepCopy( caster->GetOutput() );

   self->UpdateProgress(progressCount++/progressGoal);

   vtkNew<vtkImageMathematics> m3;
   m3->SetOperationToAdd();
   m3->SetInput1( m2->GetOutput() );
   m3->SetInput2( orig.GetPointer() );
   m3->Update();

   self->UpdateProgress(progressCount++/progressGoal);

   double range = m3->GetOutput()->GetScalarRange()[1] -
                  m3->GetOutput()->GetScalarRange()[0];

   if( range == 0. )
   {
     output->DeepCopy( input );
     return;
   }

   double m = (inputMax - inputMin) / range;
   double b = inputMax - m * m3->GetOutput()->GetScalarRange()[1];

   // scale and type match the output to the input
   //
   vtkNew<vtkImageShiftScale> shift;
   shift->SetOutputScalarType( input->GetScalarType() );
   shift->SetInputConnection( m3->GetOutputPort() );
   shift->ClampOverflowOn();
   shift->SetShift( b / m );
   shift->SetScale( m );
   shift->Update();

   self->UpdateProgress(progressCount++/progressGoal);

   output->DeepCopy( shift->GetOutput() );

   self->UpdateProgress(progressCount++/progressGoal);

   // perform histogram matching
   // compute input and output normalized histograms
   //
   int outputMin = static_cast<int>(output->GetScalarRange()[0]);
   int outputMax = static_cast<int>(output->GetScalarRange()[1]);

   int min = inputMin < outputMin ? inputMin : outputMin;
   int max = inputMax < outputMax ? outputMax : inputMax;

   std::map<int, unsigned long int> histo_source;
   std::map<int, unsigned long int> histo_target;
   for( int i = min; i <= max; i++ )
   {
     histo_source.insert( std::pair<int, unsigned long int>(i, 0L) );
   }
   histo_target.insert( histo_source.begin(), histo_source.end() );

   self->UpdateProgress(progressCount++/progressGoal);

   vtkDataArray* indata = input->GetPointData()->GetScalars();
   vtkDataArray* outdata = output->GetPointData()->GetScalars();
   std::map<int, unsigned long int>::iterator sit;
   std::map<int, unsigned long int>::iterator oit;
   int size = indata->GetNumberOfTuples();
   for( int i = 0; i < size; i++ )
   {
     sit = histo_source.find(static_cast<int>( indata->GetTuple1(i) ));
     if(sit != histo_source.end()) sit->second++;
     oit = histo_target.find(static_cast<int>( outdata->GetTuple1(i) ));
     if(oit != histo_target.end()) oit->second++;
   }

   self->UpdateProgress(progressCount++/progressGoal);

   // compute input and output cumulative distribution functions
   //
   unsigned long sum1 = 0L;
   unsigned long sum2 = 0L;
   std::map<int, unsigned long int> cdf_source;
   std::map<int, unsigned long int> cdf_target;
   sit = histo_source.begin(); 
   oit = histo_target.begin(); 
   while( sit != histo_source.end() &&  oit != histo_target.end() )
   {
      sum1 += sit->second;
      sum2 += oit->second;
      cdf_source.insert( std::pair<int, unsigned long int>(sit->first, sum1) );
      cdf_target.insert( std::pair<int, unsigned long int>(oit->first, sum2) );
      sit++;
      oit++;
   }

   self->UpdateProgress(progressCount++/progressGoal);

   // perform histogram matching on output image
   //
   for( int i = 0; i < size; i++ )
   {
     int value = static_cast<int>( outdata->GetTuple1( i ) );
     oit = cdf_target.find( value );
     if( oit != cdf_target.end() )
     {
       sit = cdf_source.find( oit->second );
       if( sit == cdf_source.end() )
       {
         sit = cdf_source.begin();
         while( sit != cdf_source.end() && sit->second < oit->second )
         {
           sit++;
         }
       } 
       if( sit != cdf_source.end() && sit->first != value )
       {
         outdata->SetTuple1( i, sit->first );
       }
     }
   }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageSharpen::SimpleExecute(vtkImageData* input,
                                    vtkImageData* output)
{
  void* inPtr = input->GetScalarPointer();
  void* outPtr = output->GetScalarPointer();

  switch(output->GetScalarType())
    {
    // This is simply a #define for a big case list. It handles all
    // data types VTK supports.
    vtkTemplateMacro(
      vtkImageSharpenExecute(this,
                             input, 
                             output,
                             static_cast<VTK_TT *>(inPtr), 
                             static_cast<VTK_TT *>(outPtr), 
                             this->StandardDeviation,
                             this->Radius,
                             this->Weight));
    default:
      vtkGenericWarningMacro("Execute: Unknown input ScalarType");
      return;
    }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageSharpen::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "Weight: " << this->Weight << endl;
  os << indent << "StandardDeviation: " << this->StandardDeviation << endl;
  os << indent << "Radius: " << this->Radius << endl;
}
