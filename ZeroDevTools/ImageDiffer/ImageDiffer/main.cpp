///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua T. Fisher
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "Support/StringMap.hpp"
#include "Support/Image.hpp"
#include "Utility/Status.hpp"
#include "Support/PngSupport.hpp"
#include "Common/Containers/Array.hpp"
#include "Math/ByteColor.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/FilePath.hpp"

void CheckStatus(Zero::Status status, Zero::StringRef filePath)
{
  if (!status)
  {
    char *message = !status ? status.Message.c_str() : "Success";

    std::cout << "Error: An image was not loaded and had this error:" << message << std::endl;
    std::cout << "File: " << filePath.c_str() << ": " << message << std::endl;
  }
}

Zero::Pair<Math::IntVec2, Math::IntVec2> 
FindPixelBoundingBox(Zero::Array<Math::IntVec2> &pixels, unsigned int width, unsigned int height)
{
  Zero::Array<Math::IntVec2>::range range = pixels.all();

  Math::IntVec2 topLeftPixel = Math::IntVec2(width, height);
  Math::IntVec2 bottomRightPixel = Math::IntVec2(0, 0);

  for (Zero::Array<Math::IntVec2>::iterator pixel = range.begin(); 
       pixel != range.end();
       ++pixel)
  {
    bottomRightPixel.x = Math::Max(pixel->x, bottomRightPixel.x);
    bottomRightPixel.y = Math::Max(pixel->y, bottomRightPixel.y);
    topLeftPixel.x = Math::Min(pixel->x, topLeftPixel.x);
    topLeftPixel.y = Math::Min(pixel->y, topLeftPixel.y);
  }

  return Zero::Pair<Math::IntVec2, Math::IntVec2>(topLeftPixel, bottomRightPixel);
}

int DoDiff(Zero::Image &image1, 
           Zero::Image &image2, 
           Zero::StringRef diffImagePath, 
           Zero::StringRef SqImagePath,
           Zero::StringRef BWImagePath,
           double mConfidence)
{
  int returnValue = 0;
  int height = image1.Height;
  int width = image1.Width;
  int resolution = height * width;
  float incorrectScore = 0.0f;

  // Allocate our difference and squared difference images.
  Zero::Image diffImage;
  diffImage.Allocate(width, height);

  Zero::Image diffSqImage;
  diffSqImage.Allocate(width, height);

  Zero::Image diffBWImage;
  diffBWImage.Allocate(width, height);

  unsigned int index = 0;
  Zero::ImagePixel *imagePixel1 = image1.Data;
  Zero::ImagePixel *imagePixel2 = image2.Data;
  Zero::ImagePixel *diffPixel = diffImage.Data;
  Zero::ImagePixel *diffSqPixel = diffSqImage.Data;
  Zero::ImagePixel *diffBWPixel = diffBWImage.Data;
  Zero::ImagePixel diffPixelColor;
  Zero::ImagePixel diffPixelSqColor;

  Zero::ImagePixel *finalPixel = imagePixel1 + resolution;

  Math::Vec4 pixelColor1;
  Math::Vec4 pixelColor2;
  Math::Vec4 diffColor;
  Math::Vec4 diffSqColor;

  Zero::Array<Math::IntVec2> wrongPixels;

  // Construct the difference and squared difference images while checking 
  // how different the images are.
  for (; imagePixel1 < finalPixel; ++index, ++imagePixel1, ++imagePixel2, ++diffPixel, ++diffSqPixel, ++diffBWPixel)
  {
    //Switch to floats for manipulation.
	  pixelColor1 = ToFloatColor(*imagePixel1);
	  pixelColor2 = ToFloatColor(*imagePixel2);

    // Find the pixel difference.
	  diffColor = pixelColor2 - pixelColor1;
	  diffColor.w = 1.0f;
	  diffColor = Math::Abs(diffColor);
    
    // Square the pixel difference.
    diffSqColor.x = Math::Sqrt(diffColor.x);
    diffSqColor.y = Math::Sqrt(diffColor.y);
    diffSqColor.z = Math::Sqrt(diffColor.z);
    diffSqColor.w = 1.0f;

    //Take our diffed colors and format them for writing out. 
    //Make sure to normalize, "just in case" - Josh Davis
    diffPixelColor = ToByteColor(Math::Clamped(diffColor, 0.0, 1.0));
    diffPixelSqColor = ToByteColor(Math::Clamped(diffSqColor, 0.0, 1.0));

    // If the difference isn't black then there's a difference.
    if (diffPixelColor == Color::Black)
    {
      *diffBWPixel = Color::Black;
    }
    else
    {
      incorrectScore += 1.0f;

      *diffBWPixel = Color::White;

      // Now we need to caclculate the position of this pixel.
      unsigned int y = index / width;
      unsigned int x = index - (y * width);

      wrongPixels.append(Math::IntVec2(x, y));
    }

    //Write out the pixels.
    *diffPixel = diffPixelColor;
    *diffSqPixel = diffPixelSqColor;
  }

  double percentageWrong = incorrectScore / resolution;
  double percentageCorrect = 1.0 - percentageWrong;

  // Check to see if the difference is small enough to pass.
  if (mConfidence > percentageCorrect)
  {
    Zero::Pair<Math::IntVec2, Math::IntVec2> boundingBox =
      FindPixelBoundingBox(wrongPixels, width, height);

    std::cout << "Warning: newImage is only " << percentageCorrect 
			        << "% correct compared to oldImage. "
              << "Bounding box of pixels: "
              << "TopLeft: (" 
              << boundingBox.first.x << ", " << boundingBox.first.y << ") "
              << "BottomRight: ("
              << boundingBox.second.x << ", " << boundingBox.second.y << ") "
              << std::endl;

	  returnValue = 1;
  }
  // Success
  else
  {
    std::cout << "Success: Images were the same!" << std::endl;
  }

  // Save out our difference and squared difference images.
  Zero::Status status;

  Zero::CreateDirectoryAndParents(Zero::FilePath::GetDirectoryPath(diffImagePath));
  SaveToPng(status, &diffImage, diffImagePath);

  CheckStatus(status, diffImagePath);

  Zero::CreateDirectoryAndParents(Zero::FilePath::GetDirectoryPath(SqImagePath));
  SaveToPng(status, &diffSqImage, SqImagePath);

  CheckStatus(status, SqImagePath);

  Zero::CreateDirectoryAndParents(Zero::FilePath::GetDirectoryPath(BWImagePath));
  SaveToPng(status, &diffBWImage, BWImagePath);

  CheckStatus(status, BWImagePath);

  
  std::cout << "Diff Image:" << diffImagePath.c_str() << std::endl;
  std::cout << "Diff Image Squared:" << SqImagePath.c_str() << std::endl;
  std::cout << "Diff Black and White:" << BWImagePath.c_str() << std::endl;

  return returnValue;
}

int main(int argc, cstr* argv)
{
  // Get our command line arguments.
  Zero::StringMap commandLineArgs;
  Zero::ParseCommandLine(commandLineArgs, argv, argc);

  Zero::String file1 = commandLineArgs.findValue("file1", "");
  Zero::String file2 = commandLineArgs.findValue("file2", "");
  
  Zero::String diffImagePath = commandLineArgs.findValue("output1", "");
  Zero::String SqImagePath = commandLineArgs.findValue("output2", "");
  Zero::String BWImagePath = commandLineArgs.findValue("output3", "");

  double confidence = Zero::GetStringValue(commandLineArgs,"confidence", 1.0);


  // Make and load our images.
  Zero::Image image1;
  Zero::Image image2;

  Zero::Status status1;
  Zero::Status status2;

  LoadFromPng(status1, &image1, file1);
  LoadFromPng(status2, &image2, file2);

  // Check if the png files loaded.
  if (!status1 || !status2)
  {
    std::cout << "Error: One or more images were not loaded:" << std::endl;

	  char *message1 = !status1 ? status1.Message.c_str() : "Success";
	  char *message2 = !status2 ? status1.Message.c_str() : "Success";

	  std::cout << file1.c_str() << ": " << message1 << std::endl;
	  std::cout << file2.c_str() << ": " << message2 << std::endl;
  }
  // Check to make sure the images are the same dimentions.
  else if ((image1.Width != image2.Width) || (image1.Height != image2.Height))
  {
    std::cout << "Error: Images are not the width and height!" << std::endl;
  }
  // Diff the images.
  else
  {
    // Print the filepaths of the images we're diffing.
    std::cout << "Original Image:" << file1.c_str() << std::endl;
    std::cout << "New Image:" << file2.c_str() << std::endl;
    DoDiff(image1, image2, diffImagePath, SqImagePath, BWImagePath, confidence);
  }

  return 0;
}
