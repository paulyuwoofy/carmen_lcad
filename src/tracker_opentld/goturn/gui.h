/*  Copyright 2011 AIT Austrian Institute of Technology
 *
 *   This file is part of OpenTLD.
 *
 *   OpenTLD is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   OpenTLD is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with OpenTLD.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GUI_H_
#define GUI_H_

#include <string>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


/**
 * Get a bounding box from the user.
 * @param img image to display
 * @param rect CvRect containing the coordinates of the bounding box
 * @param gui initialized gui
 * @return PROGRAM_EXIT if 'q' or 'Q' pressed, SUCCESS if everything went right
 */
int getBBFromUser(cv::Mat *img, cv::Rect &rect, std::string windowName);


#endif /* GUI_H_ */
