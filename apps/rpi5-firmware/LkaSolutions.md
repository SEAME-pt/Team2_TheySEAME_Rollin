# Solutions that work

Workable solutions:
- Make an histogram with the lower half of the frame.
This helps to better detect the curvature of the lanes

- Admit that a pixel gap between lanes always exists, so if the left or right
lane ever come to close or the the algorithm confuses the lanes mark as an invalid
lane
This forces the system to admit it lost one of the lanes whenever it says it found them
It can force a behavior defined by us.
The only downside is that this assumes the car is always in the middle and a gap exist between the center
to the lanes

Parameters used:
- Fov = 90, ROI = (0, 120, 640, 520), NbrPtns = 8
The camera captures 640*640 in mode 1536:864 frames

# Solutions to test more

Used solutions:
- Cut the histogram in half. The left half represents the **Left Lane** the right half represents the **Right Lane**.
This seems to help detect the **Right** and **Left** lane whenever one of them is lost. It also helps to not find the same lane 2 times
I don't know if this will work in some edge cases

- Make a virtual lane based in the points of the other lane, whenever one lane is lost.
This uses the points from one lane and it's lane size to make a guess to where the lost lane would be.
This seems to work well in scenarios where one lane is lost.
Still needs to be improved and doesn't work when the 2 lanes are lost

- Increase the number of points returned in the Sliding Windows.
This seems to better capture the curvature of the lane, but isn't enough to make the **Car** do a curve.
This seems to have diminish returns as more points are added

- Increase the width of Sliding Window rectangle.
This seems to only help in extreme cases, where the curvature of the lane is super high. In the other cases it does nothing

- Increase the Camera resolution for more lines to appear

# Ideas for Solutions

- Start the Sliding Window higher depending on the Bev fov

- Cut more than half in the histogram

- Test the wide camera mode for the camera

# Tests

Camera 2304x1296:
LKA 70fov, cropp height 220

1. 8 Lane Points, Histogram 320height (lower half), Sliding Window 150dist 80windowWidth

2. 8 Lane Points, Histogram 320height (lower half), Sliding Window 50dist 80windowWidth

3. 16 Lane Points, Histogram 425height (lower half), Sliding Window 1dist 91windowWidth

4. 16 Lane Points, Histogram 425height (lower half), Sliding Window 50dist 82windowWidth

Camera 1536x864:
LKA 150, cropp 640x230

1. 16 Lane Points, Histogram 320height (lower half), Sliding Window 1dist 91windowWidth
