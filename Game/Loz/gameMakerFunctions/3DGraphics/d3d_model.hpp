double d3d_model_create();
void d3d_model_destroy(double ind);
void d3d_model_load(double ind, const char* fname);
void d3d_model_draw(double ind, double x, double y, double z, double texid);
void d3d_model_primitive_begin(double ind, double kind);
void d3d_model_vertex_texture(double ind, double x, double y, double z, double xtex, double ytex);
void d3d_model_primitive_end(double ind);
