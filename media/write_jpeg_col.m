% Simple function to write a color image to disk
% ELEC 301 Group Project
% 11/29/2009
% Jeffrey Bridge, Robert Brockman II, Stamatios Mastrogiannis
function write_jpeg_col(A, filename)
A = min(max(A / 255,0),1);
imwrite(A, filename, 'jpeg');
return
