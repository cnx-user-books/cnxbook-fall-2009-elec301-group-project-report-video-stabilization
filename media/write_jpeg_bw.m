% Simple function to write a grayscale version of a color image to disk
% ELEC 301 Group Project
% 11/29/2009
% Jeffrey Bridge, Robert Brockman II, Stamatios Mastrogiannis
function write_jpeg_bw(A, filename)
lo = min(min(A));
hi = max(max(A));
A = (A - lo) / (hi - lo);
imwrite(A, filename, 'jpeg');
return
