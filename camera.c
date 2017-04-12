//#include <camera.h>
#include <python3.4m/Python.h>

/*
void main()
{
  Py_Initialize();
  const char name[10] = "camera";
  PyObject* myModuleString = (PyObject*) PyString_FromString(name);
  PyObject* myModule = PyImport_Import(myModuleString);
  const char name1[20] = "init_camera";
  const char name2[20] = "shoot";
  PyObject* myFunction1 = PyObject_GetAttrString(myModule,name1);
  PyObject* myFunction2 = PyObject_GetAttrString(myModule,name2);
  PyObject* args = NULL;
  PyObject_CallObject(myFunction1, args);
  PyObject_CallObject(myFunction2, args);
  Py_Finalize();
}
*/

int main() {
    Py_Initialize();
    PyRun_SimpleString("import sys; sys.path.append('.')");
    PyRun_SimpleString("import picamera");
    PyRun_SimpleString("import camera");
    PyRun_SimpleString("camera.init_camera()");
    //PyRun_SimpleString("camera.shoot()");
    Py_Finalize();

    return 0;
}