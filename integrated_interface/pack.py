import os
import platform

if (platform.system() == "Windows"):
    print 1
elif (platform.system() == "Linux"):
    dirName = 'release_' + platform.platform()
    if (not os.path.exists(dirName)):
        os.mkdir(dirName)
    #def zipdir(path, ziph):
    # ziph is zipfile handle
    #    for root, dirs, files in os.walk(path):
    #        for file in files:
    #            ziph.write(os.path.join(root, file))

    #if __name__ == '__main__':
    #    zipf = zipfile.ZipFile('Python.zip', 'w', zipfile.ZIP_DEFLATED)
    #    zipdir('tmp/', zipf)
    #    zipf.close()
    os.system('zip -r ../app.nw *')
    #os.system('cat nw app.nw > app_linux')
    #os.system('chmod a+x app_linux')
else:
    print 3
