% Simple function to load a greyscale approximation of a color image
% ELEC 301 Group Project
% 11/29/2009
% Jeffrey Bridge, Robert Brockman II, Stamatios Mastrogiannis
function A = imload_bw(filename)
A = imread(filename);
A = sum(A, 3);
return
