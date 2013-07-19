#include <vtkMedicalImageViewer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkImageActor.h>

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


int main(void)
{
  
  VTK_CREATE(vtkMedicalImageViewer, viewer );
  VTK_CREATE(vtkRenderWindowInteractor, iren);
  viewer->SetInteractor( iren );
  viewer->SetImageToSinusoid();

  std::cout << "start play" << std::endl;
  viewer->CinePlay();
  std::cout << "stop play" << std::endl;
  viewer->CineStop();
  viewer->CineRewind();
  viewer->CinePlay();
  
  std::cout << "load" << std::endl;
  viewer->Load("/home/dean/files/data/alder2/B547411/2/Cineloop/4.dcm");
 
  std::cout << "set loop on" << std::endl;
  //viewer->CineLoop(true);
  std::cout << "start play" << std::endl;
  viewer->CinePlay();

  // build buttons

  VTK_CREATE( vtkImageData, iconPlay );
  VTK_CREATE( vtkImageData, iconRev );
  VTK_CREATE( vtkImageData, iconPause );

  std::vector<std::string> iconNames;
  iconName.push_back("/home/dean/files/sandbox/animate/timePlay.png");
  iconName.push_back("/home/dean/files/sandbox/animate/timePlayReverse.png");
  iconName.push_back("/home/dean/files/sandbox/animate/pause.png");

 

  iren->Start();

  std::cout << "test end" << std::endl;
  return EXIT_SUCCESS;
}
