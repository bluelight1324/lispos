/* movefile.c -- Move File to Different Device.   Page 744  */

extern int copyfile(char *,char *,char *);

movefile(char *dst,char *src,char *mode )   /* Works like copyfile() ( see copyfile.c) */
{

   int  rval;
   rval = copyfile(dst, src, mode); 
   return rval;
}
