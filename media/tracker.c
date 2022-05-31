/**********************************************************************
Finds the 100 best features in an image, and tracks these
features to the next image.  Saves the feature
locations (before and after tracking) to text files and to PPM files, 
and prints the features to the screen.
**********************************************************************/

#include "pnmio.h"
#include "klt.h"
#include <stdlib.h>

#include "im2_jpeg.c"

int main(int argc,char *argv[])
{
	int nFeatures, nFrames;
        if (argc != 4) {
            printf("Usage: %s nFeatures nFrames prefix\n",argv[0]);
            return 1;
        }
        nFeatures = atoi(argv[1]); /* convert strings to integers */
        nFrames = atoi(argv[2]);
        printf("%s received nFeatures=%i nFrames=%i prefix=%s\n",argv[0],nFeatures
	       ,nFrames,argv[3]);

	unsigned char *img1, *img2;
	KLT_TrackingContext tc;
	KLT_FeatureList fl;
	KLT_FeatureTable ft;

	int ncols, nrows;
	int i, imgnum;

	tc = KLTCreateTrackingContext();

	KLTPrintTrackingContext(tc);

	fl = KLTCreateFeatureList(nFeatures);
	ft = KLTCreateFeatureTable(nFrames, nFeatures);
	tc->sequentialMode = TRUE;

	char filenamebuff[300];
	sprintf(filenamebuff, "%s%08d.jpg", argv[3],1);
	img1 = image_load_jpeg(filenamebuff, 0, 0, &ncols, &nrows, NULL);
	
	KLTSelectGoodFeatures(tc, img1, ncols, nrows, fl);
	KLTStoreFeatureList(fl, ft, 0);

	for(imgnum = 2; imgnum <= nFrames; imgnum++) {
		printf("now tracking image %d...\n", imgnum);

		char filenamebuff[300];
		char outputnamebuff[300];
		sprintf(filenamebuff, "%s%08d.jpg", argv[3],imgnum);
		img2 = image_load_jpeg(filenamebuff, 0, 0, &ncols, &nrows, NULL);
		
		// fl corresponds to img1 before tracking
		KLTTrackFeatures(tc, img1, img2, ncols, nrows, fl);
		// fl corresponds to img2 now


		KLTReplaceLostFeatures(tc, img2, ncols, nrows, fl);
		KLTStoreFeatureList(fl, ft, imgnum-1);

		/* Cleanup and cycling */
		free(img1);
		img1 = img2;	
	}
	KLTWriteFeatureTable(ft, "features.txt", "%5.1f");
	return 0;
}
