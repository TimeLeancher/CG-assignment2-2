## CG_Assignment_2_Q2
---
About
===
Gamma Correction   

---
How to Use

1. Download zip piles.
   
![download zip](https://github.com/user-attachments/assets/3e76e9d2-5325-42a3-ba52-2bb3064c0a58)

2. Unzip the folder
3. open "OpenglViewer.sln"
![leanch](https://github.com/user-attachments/assets/1ed43ef3-d812-4b75-809d-fe1077eabf9b)
---
Result of assignmet  
---   
![result2](https://github.com/user-attachments/assets/a39519f6-bc6d-4ffe-be26-a8bfd2e5e44b)

---
Explanation
---
Q2 implements the ability to gamma-correct the color of the rendered image and display it correctly on the display device.

Modify render() function:   
![render](https://github.com/user-attachments/assets/91d0dd09-3a3d-4cde-b830-2c4791abbe4b)  
After calculating the color of each pixel, we apply the gamma correction formula.  
![gamma](https://github.com/user-attachments/assets/404ac467-8f07-4482-b72f-3f070d802084)

Calculate the values of color.r, color.g, and color.b as 1/gamma multiplication using the power() function.  
