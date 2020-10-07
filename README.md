# Edge Extraction


In computer science, images are 2D arrays:
> width x height (grayscale)

> width x height for (3 color values)


In mathematics, they are functions sampled on a 2D-grid:
> f(x,y): R2 → R (grayscale)

> f(x,y): R2 → R3 (3 color values)


Filtering:
It aims to form new images whose pixels are a combination of the original pixels in order to either get useful information (ex: extract edges to understand shapes, ...) or to enhance the images (ex: remove noise, ...). 


Edge detection:
The goal of edge detection is to identify sudden changes or discontinuities in an image. The edge points carry important semantic and shape information and it is interesting to notice that even young children have no difficulty in recognizing familiar objects from such simple line drawings. Thus edge detection is a way of extracting information and recognizing objects more easily. It must be noted that keeping only the edges of an image is also more compact.

How can we find salient edges in an image?
An edge occurs when the color of the image changes, hence the intensity of the pixel changes as well. Segmenting an image into colored regions is a very difficult task and it is often preferable to detect edges using only purely local information. Thus, a reasonable approach is to define an edge as a location of rapid intensity variation. As we said earlier images can be thought as mathematical functions sampled on a 2D-grid and therefore locations of steep slopes correspond to eges locations.

To define the slope and direction of steepest ascent in the intensity function we use its gradient: J(x) = ∇I(x) = (∂I/∂x, ∂I/∂y)(x)
Its magnitude indicates the slope of the intensity variation, while its orientation indicates the perpendicular direction to the local contour.


## Canny edge detection:
Canny edge detection is an image processing method used to detect edges in an image while suppressing noise. First it convert the image into grayscale. Then it computes gradient using a Sobel filter. The magnitude and the angle of the gradient will be used for nonmaximum suppresion. 
The result of the filter is an image with thick edges but ideally it should have thin edges. Thus, we must perform non maximum suppression to thin out the edges.
Non maximum suppression works by finding the pixel with the maximum value in an edge. It occurs when a pixel has an intensity that is larger than its 2 neighbors in the gradient direction. If this condition is true, then we set the pixel to 255 (white), otherwise we set it to 0 (black).


After this step, the result is still not perfect: some edges may not actually be edges and there is some noise in the image. Hence the reason we use double thresholding. We set two thresholds, a high and a low threshold. Pixels with a high value are most likely to be edges. For example, you might choose the high threshold to be 0.7, this means that all pixels with a value larger than 0.7 will be a strong edge set to 255 (white). You might also choose a low threshold of 0.3, this means that all pixels less than it is not an edge and you would set it to 0 (black). The values in between 0.3 and 0.7 would be weak edges, in other words, we do not know if these are actual edges or not edges at all. 
Now that we have determined what the strong edges and weak edges are, we need to determine which weak edges are actual edges. To do this, we perform an edge tracking algorithm. Weak edges that are connected to strong edges will be actual edges. Weak edges that are not connected to strong edges will not beactual edges and will be removed. 


