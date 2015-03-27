////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//Created by Himanshu Chablani



namespace octet {

  struct stk
  {
    mat4t _branchtotrunktransform;
    uint32_t _basevert;
    float _rad;
    float _exclen;
    stk *next;
  };
  class L_stack
  {
    stk *head;
    public:
      L_stack(float rad, float exelen, mat4t dir)
      {
        head = new stk();
        head->_rad = rad;
        head->_exclen = exelen;
        head->_branchtotrunktransform = dir;
        head->_basevert=0;
        head->next = NULL;
      }        
      void push(uint32_t base, float rad, float exelen, mat4t dir)
      {
        stk *temp = new stk();
        temp->_rad = rad;
        temp->_basevert = base;
        temp->_exclen = exelen;
        temp->_branchtotrunktransform = dir;
        temp->next = head;
        head = temp;
      }
      void eraseall()
      {
        while (head!=NULL)
        { 
        stk * temp = head;
        head = head->next;
        delete temp;
        }
      }
      void pop()
      {       
        stk * temp = head;        
        head = head->next;
        delete temp;
      }
      
      void gethead(uint32_t &base, float &rad, float &exelen, mat4t &dir)
      {
        
        base = head->_basevert;
        rad = head->_rad;
        exelen = head->_exclen;
        dir = head->_branchtotrunktransform;
      }      

      void sethead(uint32_t base,float rad,float exelen, mat4t dir)
      {
        head->_rad = rad;
        head->_basevert=base;
        head->_exclen = exelen;
        head->_branchtotrunktransform = dir;
      }      
  };

  struct my_vertex {
    vec3p pos;
    vec3p nor;
    uint32_t color;
  };  
  static uint32_t make_color(float r, float g, float b) {
    return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
  }
  
  class Maths_L_Systems : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;
    /** @var  ref<visual_scene> app_scene
    *   @brief The visual scene 
    */
    FILE *L_Rules;
    /** @var  FILE *L_Rules
    *   @brief The file pointer that we read from
    */
    char* current_param;
    /** @var  char* current_param
    *   @brief The L-System the we are currently making
    */
    int number_of_rules;
    /** @var  int number_of_rules
    *   @brief The number of rules this L-System has
    */
    float angle;
    /** @var  float angle
    *   @brief The angle of rotation for the + and - of the L_System
    */
    float scale;
    /** @var  float scale
    *   @brief The scaling factor
    */
    char symbols[5];
    /** @var  char symbols[5]
    *   @brief The symbols on the current L-System
    */
    hash_map<char, char *> rules;
    /** @var  hash_map<char, char *> rules
    *   @brief The map between the symbols and the rules
    */
    L_stack *var_stack;
    /** @var  L_stack *var_stack
    *   @brief The stack variable in which we push and pop the states
    */
    float rad ;
    /** @var  float rad
    *   @brief The radious of the circle
    */
    float exel ;
    /** @var  float exel
    *   @brief The extrution length for the cyln
    */
    scene_node *node;
    /** @var  scene_node *node
    *   @brief The node for the mesh
    */
    float initrad;
    /** @var  float initrad
    *   @brief The initial radious
    */
    float initexel;
    /** @var  float initexel
    *   @brief The initial extrution length
    */
    int curr_itr;    
    /** @var  int curr_itr
    *   @brief The current iteration number
    */
    bool done;
    /** @var  bool done
    *   @brief true if the tree is build
    */
    char* init_param;
    /** @var  char* init_param
    *   @brief the initial paramiter for the system
    */
    //day night cycle
    bool isday;
    /** @var  bool isday
    *   @brief true is its day, false if night
    */
    int nightnumber;
    /** @var  int nightnumber
    *   @brief the number of nights that have passed
    */
    bool enemiesrecycled;
    /** @var  bool enemiesrecycled
    *   @brief true if the enemies have been placed in the game , false if not (only used a trigger)
    */
    float gamedaytime;
    /** @var   float gamedaytime
    *   @brief The game day night time (0-360)
    */
    scene_node *daynightnode;
    /** @var   scene_node *daynightnode
    *   @brief The scene node for the day night cycle objects (sun and moon)
    */

    param_shader *shader ;
    /** @var   param_shader *shader
    *   @brief The shader for the tree
    */
    material *mat_tree ;
    /** @var   material *mat_tree
    *   @brief The material for the tree
    */
    vec3 matrixmult(mat4t objm, vec3 direction)
    {
      vec3 temp(objm.x().x()*direction.x() + objm.y().x()*direction.y() + objm.z().x()*direction.z(),
        objm.x().y()*direction.x() + objm.y().y()*direction.y() + objm.z().y()*direction.z(),
        objm.x().z()*direction.x() + objm.y().z()*direction.y() + objm.z().z()*direction.z());
      return temp;
    }
    /** @fn  vec3 matrixmult(mat4t objm, vec3 direction)
    *   @brief This function is used to calu the direction of the camera
    */
    void user_cont()
    {
      vec3 forworddir = matrixmult(app_scene->get_camera_instance(0)->get_node()->access_nodeToParent(), vec3(0, 0, -1));
      vec3 backdir = matrixmult(app_scene->get_camera_instance(0)->get_node()->access_nodeToParent(), vec3(0, 0, 1));
      vec3 rightdir = matrixmult(app_scene->get_camera_instance(0)->get_node()->access_nodeToParent(), vec3(1, 0, 0));
      vec3 leftdir = matrixmult(app_scene->get_camera_instance(0)->get_node()->access_nodeToParent(), vec3(-1, 0, 0));
      if (is_key_down(key::key_up))
      {
        app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(15 * forworddir.x(), 15 * forworddir.y(), 15 * forworddir.z());
      }
      if (is_key_down(key::key_left))
      {
        app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(15 * leftdir.x(), 15 * leftdir.y(), 15 * leftdir.z());
      }
      if (is_key_down(key::key_down))
      {
        app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(15 * backdir.x(), 15 * backdir.y(), 15 * backdir.z());
      }
      if (is_key_down(key::key_right))
      {
        app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(15 * rightdir.x(), 15 * rightdir.y(), 15 * rightdir.z());
      }

      if (done)
      {
      if (is_key_down(key::key_f1))
      {
        done =false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(0);        
        iter(curr_itr);
      }
      if (is_key_down(key::key_f2))
      {
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(1);
        iter(curr_itr);
      }
      if (is_key_down(key::key_f3))
      {
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(2);
        iter(curr_itr);
      }
      if (is_key_down(key::key_f4))
      {
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(3);
        iter(curr_itr);
      }
      if (is_key_down(key::key_f5))
      {
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(4);
        iter(curr_itr);
      }
      if (is_key_down(key::key_f6))
      {
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(5);
        iter(curr_itr);
      }
      if (is_key_down(key::key_f7))
      {
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(6);
        iter(curr_itr);
      }
      if (is_key_down(key::key_f8))
      { 
        done = false;
        curr_itr = 1;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        getrule(7);
        iter(curr_itr);
      }

      if (is_key_down(key::key_lmb))
      {     
        if (curr_itr<7)
        {         
        done = false;
        curr_itr++;
        //printf("\nThe current ITR _IS : %d", curr_itr);
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);       
        
        iter(curr_itr);
        }
      }
      if (is_key_down(key::key_rmb))
      {
        if (curr_itr>1)
        {          
          done = false;
          curr_itr--;
          //printf("\nThe current ITR _IS : %d", curr_itr);
          rad = initrad;
          exel = initexel;
          mat4t dir;
          dir.loadIdentity();
          dir.translate(0.0f, -10.0f, 0.0f);
          var_stack = new L_stack(rad, exel, dir);
          
          iter(curr_itr);
        }
      }

      if (is_key_down(key::key_alt))
      {
        done = false;
        rad = initrad;
        exel = initexel;
        angle--;
        
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        iter(curr_itr);
      }

      if (is_key_down(32))
      {
        done = false;
        rad = initrad;
        exel = initexel;
        angle++;
        
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);       
        iter(curr_itr);
      }

      if (is_key_down(key::key_home))
      {
        done = false;
        initrad +=1.0f;
        rad = initrad;
        exel = initexel;        
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        iter(curr_itr);
      }

      if (is_key_down(key::key_end))
      {
        done = false;
        if (initrad>1.0f)
        initrad -= 1.0f;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        iter(curr_itr);
      }

      if (is_key_down(key::key_insert))
      {
        done = false;
        initexel+= 1.0f;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        iter(curr_itr);
      }
      if (is_key_down(key::key_delete))
      {
        if (initexel>1.0f)
        initexel -= 1.0f;
        done = false;
        rad = initrad;
        exel = initexel;
        mat4t dir;
        dir.loadIdentity();
        dir.translate(0.0f, -10.0f, 0.0f);
        var_stack = new L_stack(rad, exel, dir);
        iter(curr_itr);
      }
      }
      if (is_key_down(key::key_esc))
      {
        exit(1);
      }

      if (is_key_down(key::key_shift))
      {
        app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(0, 15 , 0);
      }
      if (is_key_down(key::key_ctrl))
      {
        app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(0,-15, 0);
      }
      
      int vx = 0, vy = 0;
      int x, y;
      get_viewport_size(vx, vy);
      get_mouse_pos(x, y);
           
      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld[3] = vec4(app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().w().x(), app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().w().y(), app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().w().z(), 1);
      modelToWorld.rotateY((float)-x*2.0f);
      if (vy / 2 - y < 40 && vy / 2 - y > -40)
        modelToWorld.rotateX(vy / 2 - y);
      if (vy / 2 - y >= 40)
        modelToWorld.rotateX(40);
      if (vy / 2 - y <= -40)
        modelToWorld.rotateX(-40);
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent() = modelToWorld;//apply to the node
      
    }
    /** @fn  void user_cont()
    *   @brief The user controls
    */
    int countnumofstep()
    {
      int temp=0;
      for (int i = 0; current_param[i] != '\0'; i++)
      {
        switch (current_param[i])
        {
        case '[':
          break;
        case ']':
          break;
        case '+':
          break;
        case '-':
          break;
        default:
                temp++;
          break;
        }
      }
      return temp;
    }
    /** @fn  int countnumofstep()
    *   @brief This function is used to count the number of variable Symbols in the prarmiter to know the number of verts and indices
    */
    int countnumofbranches()
    {
      int temp = 0;
      for (int i = 0; current_param[i] != '\0'; i++)
      {
        switch (current_param[i])
        {
        case '[':
          break;
        case ']':
          break;
        case '+':
          temp++;
          break;
        case '-':
          temp++;
          break;
        default:
          
          break;
        }
      }
      return temp;    
    }
    /** @fn  int countnumofbranches()
    *   @brief This function is used to count the number of [] in the prarmiter to know the number of verts and indices
    */
    void interp_param()
    {
      var_stack->eraseall();               
      float curr_angle=angle;   
      //printf("\n angle used : %f",curr_angle);
      uint32_t current_circle_vrt = 0;
      uint32_t base_circle_vrt = 0;
      mat4t dir;
      dir.loadIdentity();
      //dir.rotateX(90);
      //vec3 lookdir = matrixmult(dir, vec3(0, 0, -1));
      dir.translate(0.0f, -5.0f, 0.0f);      

      var_stack = new L_stack(rad,exel,dir);
      uint32_t tempcurre = current_circle_vrt;
      mat4t tempdir;
      
      mesh *tree = new mesh();
      size_t num_steps = countnumofstep();
      size_t numofbran = countnumofbranches();
      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = num_steps*10+20*numofbran+10;
      size_t num_indices = 6*num_vertices;//num_vertices*10;
      tree->allocate(sizeof(my_vertex) * num_vertices, sizeof(uint32_t) * num_indices);
      tree->set_params(sizeof(my_vertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);
      // describe the structure of my_vertex to OpenGL
      tree->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      tree->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);      
        gl_resource::wolock vl(tree->get_vertices());
        my_vertex *vtx = (my_vertex *)vl.u8();
        gl_resource::wolock il(tree->get_indices());
        uint32_t *idx = idx = il.u32();      
        
        if (app_scene->get_num_mesh_instances() > 4)//make this more if adding mre meshes to the scene
        { 
        app_scene->pop_mesh_instance();
        }
        //make the base        
        for (size_t i = 0; i<10; ++i) {
          float r = 0.0f, g = 1.0f, b = 0.0f;
          float angle = i *(2.0f * 3.14159265f / 10.0f);
          float y = dir.w().y();
          float x = dir.w().x() + cosf(angle) * rad;
          float z = dir.w().z() + sinf(angle) * rad;
         // printf("\n\n The position 0 is : %f %f %f ", x, y, z);
          vtx->pos = vec3p(x, y,z); 
          vtx->nor = vec3p(x, y, z);
          //pos_vert[i]=(vec3(x,y,z));
          vtx->color = make_color(r, g, b);
          vtx++;                 
        }     
        dir.translate(0.0f, exel, 0.0f);
        if (rad > 3.0f)
          rad -= 0.02f;
        if (exel > 8.0f)
          exel -= 0.02f;
        var_stack->sethead(base_circle_vrt, rad, exel, dir);
        
      for (int i=0;current_param[i]!='\0';i++)
      {
        switch (current_param[i])
        {
          case '[':
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            var_stack->push(base_circle_vrt, rad, exel, dir);
          break;
          case ']':
            var_stack->pop();
          break;
          case '+':   
            //var_stack->pop();
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            for (size_t i = 0; i<10; ++i) {
              float r = 0.0f, g = 1.0f, b = 0.0f;
              // basepos = pos_vert[base_circle_vrt];
              float angle = i *(2.0f * 3.14159265f / 10.0f);
              float y = dir.w().y();
              float x = dir.w().x() + cosf(angle) * rad;
              float z = dir.w().z() + sinf(angle) * rad;
              //printf("\n\n The position 1 is : %f %f %f ", x, y, z);
              vtx->pos = vec3p(x, y, z);
              vtx->nor = vec3p(x, y, z);
              //pos_vert[current_circle_vrt + 10] = vec3(x, y, z);
              vtx->color = make_color(r, g, b);
              vtx++;
              current_circle_vrt++;
              base_circle_vrt++;
            }
            // make the triangles  
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            tempcurre = current_circle_vrt;
            for (size_t i = 0; i <9; ++i) {
              //10--11
              // |/ |
              // 0--1
              idx[0] = tempcurre;
              idx[1] = tempcurre + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;

              idx[0] = tempcurre + 1;
              idx[1] = base_circle_vrt + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;
              tempcurre += 1;
              base_circle_vrt += 1;
            }
            tempcurre += 1;
            base_circle_vrt += 1;
            idx[0] = current_circle_vrt;
            idx[1] = current_circle_vrt + 9;
            idx[2] = base_circle_vrt - 1;
            idx += 3;
            idx[0] = current_circle_vrt;
            idx[1] = base_circle_vrt - 10;
            idx[2] = base_circle_vrt - 1;
            idx += 3;
            //printf("\ncurrent vert : %i", current_circle_vrt);

            //make the left verts
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            dir.rotateZ(curr_angle);
            dir.translate(0.0f, exel, 0.0f);
            //printf("\n base vert : %i", base_circle_vrt);
            for (size_t i = 0; i<10; ++i) {
              float r = 0.0f, g = 1.0f, b = 0.0f;
              float angle = i *(2.0f * 3.14159265f / 10.0f);

              float y = dir.w().y();
              float x = dir.w().x() + cosf(angle) * rad;
              float z = dir.w().z() + sinf(angle) * rad;

              //printf("\n\n The position is : %f %f %f ", x, y, z);
              vtx->pos = vec3p(x, y, z);
              vtx->nor = vec3p(x, y, z);
              vtx->color = make_color(r, g, b);
              vtx++;
              current_circle_vrt++;
              base_circle_vrt++;
            }
            //printf("\ncurrent vert : %i", current_circle_vrt);
            //make the triangles
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            //printf("\n base vert : %i", base_circle_vrt);
            tempcurre = current_circle_vrt;
            for (int i = 0; i <9; ++i) {
              //10--11
              // |/ |
              // 0--1
              idx[0] = tempcurre;
              idx[1] = tempcurre + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;

              idx[0] = tempcurre + 1;
              idx[1] = base_circle_vrt + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;
              tempcurre += 1;
              base_circle_vrt += 1;
            }
            tempcurre += 1;
            base_circle_vrt += 1;
            idx[0] = current_circle_vrt;
            idx[1] = current_circle_vrt + 9;
            idx[2] = base_circle_vrt - 1;
            //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
            idx += 3;
            idx[0] = current_circle_vrt;
            idx[1] = base_circle_vrt - 10;
            idx[2] = base_circle_vrt - 1;
            //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
            idx += 3;

            var_stack->gethead(base_circle_vrt, rad, exel, dir);            
            dir.translate(0.0f, exel, 0.0f);
            var_stack->sethead(base_circle_vrt + 10, rad, exel, dir);

            dir.translate(0.0f, -exel, 0.0f);
            tempdir = dir;
            tempdir.rotateZ(curr_angle);
            tempdir.translate(0.0f, exel, 0.0f);            
            var_stack->sethead(base_circle_vrt + 10, rad, exel, tempdir);
            //var_stack->push(current_circle_vrt, rad, exel, tempdir);
          break;

          case '-':            
              
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            for (size_t i = 0; i<10; ++i) {
              float r = 0.0f, g = 1.0f, b = 0.0f;
              // basepos = pos_vert[base_circle_vrt];
              float angle = i *(2.0f * 3.14159265f / 10.0f);
              float y = dir.w().y();
              float x = dir.w().x() + cosf(angle) * rad;
              float z = dir.w().z() + sinf(angle) * rad;
              //printf("\n\n The position 1 is : %f %f %f ", x, y, z);
              vtx->pos = vec3p(x, y, z);
              vtx->nor = vec3p(x, y, z);
              //pos_vert[current_circle_vrt + 10] = vec3(x, y, z);
              vtx->color = make_color(r, g, b);
              vtx++;
              current_circle_vrt++;
              base_circle_vrt++;
            }
            // make the triangles  
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            tempcurre = current_circle_vrt;
            for (size_t i = 0; i <9; ++i) {
              //10--11
              // |/ |
              // 0--1
              idx[0] = tempcurre;
              idx[1] = tempcurre + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;

              idx[0] = tempcurre + 1;
              idx[1] = base_circle_vrt + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;
              tempcurre += 1;
              base_circle_vrt += 1;
            }
            tempcurre += 1;
            base_circle_vrt += 1;
            idx[0] = current_circle_vrt;
            idx[1] = current_circle_vrt + 9;
            idx[2] = base_circle_vrt - 1;
            idx += 3;
            idx[0] = current_circle_vrt;
            idx[1] = base_circle_vrt - 10;
            idx[2] = base_circle_vrt - 1;
            idx += 3;
            //printf("\ncurrent vert : %i", current_circle_vrt);

            //make the left verts
            var_stack->gethead(base_circle_vrt, rad, exel, dir);           
            dir.rotateZ(-curr_angle);
            dir.translate(0.0f, exel, 0.0f);
            //printf("\n base vert : %i", base_circle_vrt);
            for (size_t i = 0; i<10; ++i) {
              float r = 0.0f, g = 1.0f, b = 0.0f;
              float angle = i *(2.0f * 3.14159265f / 10.0f);

              float y = dir.w().y();
              float x = dir.w().x() + cosf(angle) * rad;
              float z = dir.w().z() + sinf(angle) * rad;

              //printf("\n\n The position is : %f %f %f ", x, y, z);
              vtx->pos = vec3p(x, y, z);
              vtx->nor = vec3p(x, y, z);
              vtx->color = make_color(r, g, b);
              vtx++;
              current_circle_vrt++;
              base_circle_vrt++;
            }
            //printf("\ncurrent vert : %i", current_circle_vrt);
            //make the triangles
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            //printf("\n base vert : %i", base_circle_vrt);
            tempcurre = current_circle_vrt;
            for (int i = 0; i <9; ++i) {
              //10--11
              // |/ |
              // 0--1
              idx[0] = tempcurre;
              idx[1] = tempcurre + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;

              idx[0] = tempcurre + 1;
              idx[1] = base_circle_vrt + 1;
              idx[2] = base_circle_vrt;
              //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
              idx += 3;
              tempcurre += 1;
              base_circle_vrt += 1;
            }
            tempcurre += 1;
            base_circle_vrt += 1;
            idx[0] = current_circle_vrt;
            idx[1] = current_circle_vrt + 9;
            idx[2] = base_circle_vrt - 1;
            //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
            idx += 3;
            idx[0] = current_circle_vrt;
            idx[1] = base_circle_vrt - 10;
            idx[2] = base_circle_vrt - 1;
            //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
            idx += 3;

            var_stack->gethead(base_circle_vrt, rad, exel, dir);            
            dir.translate(0.0f, exel, 0.0f);
            var_stack->sethead(base_circle_vrt + 10, rad, exel, dir);

            dir.translate(0.0f, -exel, 0.0f);
            tempdir = dir;
            tempdir.rotateZ(-curr_angle);
            tempdir.translate(0.0f, exel, 0.0f);             
            var_stack->sethead(base_circle_vrt + 10, rad, exel, tempdir);
            //var_stack->push(current_circle_vrt, rad, exel, tempdir);
          break;   
                 
          default:
            var_stack->gethead(base_circle_vrt, rad, exel, dir);
            if (rad > 3.0f)              
              rad -= 0.02f;
            if (exel > 8.0f)
              exel -= 0.02f;            
        for (size_t i = 0; i<10; ++i) {
          float r = 0.0f, g = 1.0f, b = 0.0f;
         // basepos = pos_vert[base_circle_vrt];
          float angle = i *(2.0f * 3.14159265f / 10.0f);
          float y = dir.w().y();
          float x = dir.w().x() + cosf(angle) * rad;
          float z = dir.w().z() + sinf(angle) * rad;
         // printf("\n\n The position 1 is : %f %f %f ", x, y, z);
          vtx->pos = vec3p(x, y, z);
          vtx->nor = vec3p(x, y, z);
         //pos_vert[current_circle_vrt + 10] = vec3(x, y, z);
          vtx->color = make_color(r, g, b);
          vtx++;
          current_circle_vrt++;
          base_circle_vrt++;
        }
        // make the triangles  
        var_stack->gethead(base_circle_vrt, rad, exel, dir);
        tempcurre = current_circle_vrt;
        for (size_t i = 0; i <9; ++i) {
          //10--11
          // |/ |
          // 0--1
          idx[0] = tempcurre;
          idx[1] = tempcurre + 1;
          idx[2] = base_circle_vrt;
          //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
          idx += 3;

          idx[0] = tempcurre + 1;
          idx[1] = base_circle_vrt + 1;
          idx[2] = base_circle_vrt;
          //printf("\n\n The triangle formed is : %i,%i,%i", idx[0], idx[1], idx[2]);
          idx += 3;
          tempcurre += 1;
          base_circle_vrt += 1;
        }
        tempcurre += 1;
        base_circle_vrt += 1;
        idx[0] = current_circle_vrt;
        idx[1] = current_circle_vrt + 9;
        idx[2] = base_circle_vrt - 1;
        idx += 3;
        idx[0] = current_circle_vrt;
        idx[1] = base_circle_vrt - 10;
        idx[2] = base_circle_vrt - 1;
        idx += 3;
        dir.translate(0.0f, exel, 0.0f);
        var_stack->sethead(current_circle_vrt, rad, exel, dir);
            break;        
        }
        
      }     
      
      node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, tree, mat_tree));
      done = true;
     // node->rotate(45,vec3(0,1,0));
    }
    /** @fn  void interp_param()
    *   @brief This function converts the paramiter into the mesh
    */
    void getrule(int rule_number)
    {
      char str[200];
      int current_rule_number = 0;
      number_of_rules=0;
      angle=0;
      current_param = new char[1000000];
      init_param = new char[20];
      fseek(L_Rules,0,0);      
      while (!feof(L_Rules))
      {
        char initsym = fgetc(L_Rules);//chk for the symbols
        switch (initsym)
        {
        case 'S'://printf("Symbol's");
          initsym = fgetc(L_Rules);//remove the :
                    
          if (current_rule_number == rule_number)
          {
            symbols[number_of_rules] = fgetc(L_Rules);
            number_of_rules++;
          }
          else
          {
            initsym = fgetc(L_Rules);//remove the sym
          }
          initsym = fgetc(L_Rules);//remove the \n
          break;
        case 'B'://printf("Beging symbol");
          initsym = fgetc(L_Rules);//remove the :
          if (current_rule_number == rule_number)
          {
            fscanf(L_Rules, "%s", str);
            strcpy(current_param,str);
            strcpy(init_param,str);
          }
          else
          {
            fscanf(L_Rules, "%s", str);//remove the sym
          }
          initsym = fgetc(L_Rules);//remove the \n
          break;
        case 'R'://printf("Rules's");
          initsym = fgetc(L_Rules);//remove the :                    
          if (current_rule_number == rule_number)
          {
            char sym = fgetc(L_Rules);
            char intdent = fgetc(L_Rules);
            fscanf(L_Rules, "%s", str);
            rules[sym] = new char[15];
            strcpy(rules[sym],str);
          }
          else
          {
            fscanf(L_Rules, "%s", str);//remove the sym
          }
          initsym = fgetc(L_Rules);//remove the \n
          break;
        case 'A'://printf("angle's");
          initsym = fgetc(L_Rules);//remove the :         
          if (current_rule_number == rule_number)
          {
            fscanf(L_Rules, "%s", str);            
            int lenofarr = strlen(str);
            angle =0.0f;

            for (int i = 0; str[i]!='.'; i++)
            { 
              angle += (str[i]-48);
              if (str[i+1]!='.')
              angle*=10;
            }
            angle+= (str[lenofarr - 1] - 48) / 10.0f;
          }
          else
          {
            fscanf(L_Rules, "%s", str);
            //printf("%s\n", str);
          }
          initsym = fgetc(L_Rules);//remove the \n
          break;
        case '/'://printf("///Next Rule");
          fscanf(L_Rules, "%s", str);
          //printf("%s\n", str);
          initsym = fgetc(L_Rules);//remove the \n
          current_rule_number++;
          if (current_rule_number>rule_number)
          {
            return;
          }
          break;
        }

      }
    }
    /** @fn  void getrule(int rule_number)
    *   @brief This function is used to fetch the rule from the file
    */
    void iter(int times)
    {
      char *temp = new char[1000000];
      int flag=0;
      char tempcpy;
      strcpy(current_param, init_param);
      for (int i = 0; i<times; i++)
      {       
        strcpy(temp,"");        
        for (int j = 0; j<strlen(current_param); j++)
        {
          flag =0;
          for (int z=0;z<number_of_rules;z++)
          {
            if (current_param[j] == symbols[z])
          {      
            flag=1;     
            temp = strcat(temp, rules[symbols[z]]);            
          }
          }
          if (flag==0)
          {
            tempcpy = current_param[j];
            int lenb4copy = strlen(temp);
            temp[lenb4copy++] = tempcpy;
            temp[lenb4copy] = '\0';
          }
          //printf("\n the temp : %s \n", temp);
        } 
        strcpy(current_param, temp);
        //printf("\n param : %s\n",current_param);
      }
      interp_param();
    }
    /** @fn  void iter(int times)
    *   @brief This function itirates the current paramiter the number of times indicated and then builds the tree
    */

    void create_base()
    {
      mat4t modelToWorld;

      modelToWorld.loadIdentity();
      modelToWorld.translate(0, -100, 0);
      mesh_box *box = new mesh_box(vec3(22000, 100, 22000));
      scene_node *node = new scene_node(modelToWorld, atom_);
      app_scene->add_child(node);
      material *floor_mat = new material(new image("assets/ground.gif"));
      app_scene->add_mesh_instance(new mesh_instance(node, box, floor_mat));
    }
    /** @fn  void create_base()
    *   @brief This function creats the base for the scene
    */
    void InitDayNightCycle()
    {
      mat4t modelToWorld;
      mat4t modelToWorld_sun;
      mat4t modelToWorld_moon;
      mat4t modelToWorld_sky;
      mat4t modelToWorld_sunlight;

      modelToWorld_sunlight.loadIdentity();
      modelToWorld_sunlight.translate(4900, 0, 0);
      modelToWorld_sunlight.rotateY(-90);

      modelToWorld_sky.loadIdentity();
      modelToWorld.loadIdentity();
      modelToWorld_sun.loadIdentity();
      modelToWorld_sun.translate(19000, 0, 0);
      modelToWorld_moon.loadIdentity();
      modelToWorld_moon.translate(-19000, 0, 0);
      daynightnode = new scene_node(modelToWorld, atom_);
      app_scene->add_child(daynightnode);
      scene_node *sky_node = new scene_node(modelToWorld_sky, atom_);
      scene_node *sun_node = new scene_node(modelToWorld_sun, atom_);
      scene_node *moon_node = new scene_node(modelToWorld_moon, atom_);
      scene_node *sunlight_node = new scene_node(modelToWorld_sunlight, atom_);

      mesh_sphere *sky = new mesh_sphere(vec3(0, 0, 0), 20000);
      mesh_sphere *sun = new mesh_sphere(vec3(19000, 0, 0), 500);
      mesh_sphere *moon = new mesh_sphere(vec3(-19000, 0, 0), 500);
            
      material *sun_mat = new material(vec4(0.992, 0.721, 0.074, 1));
      material *moon_mat = new material(vec4(0.266, 0.388, 0.956, 1));
      material *sky_mat = new material(vec4(0.247, 0.3039, 0.580, 1));
      app_scene->add_mesh_instance(new mesh_instance(sun_node, sun, sun_mat));
      app_scene->add_mesh_instance(new mesh_instance(moon_node, moon, moon_mat));
      app_scene->add_mesh_instance(new mesh_instance(sky_node, sky, sky_mat));
      daynightnode->add_child(sun_node);
      daynightnode->add_child(moon_node);
    }
    /** @fn  void InitDayNightCycle()
    *   @brief This function is used to init the day night cycle
    */
    void DayNightCycle()
    {
      if (!isday && !enemiesrecycled)
      {
        nightnumber++;
      }

      daynightnode->access_nodeToParent().rotateZ(0.1f);
      app_scene->get_light_instance(0)->get_node()->access_nodeToParent().rotateX(-0.1f);
      gamedaytime += 0.1f;

      if (gamedaytime > 360)
      {
        gamedaytime -= 360;
      }
      if (gamedaytime > 0 && gamedaytime < 200 && !isday)
      {
        isday = true;
       
      }
      if (gamedaytime < 360 && gamedaytime>200 && isday)
      {
        isday = false;
      }
    }
    /** @fn  void DayNightCycle()
    *   @brief This function deals with the actual rotation of the dynightcycle node 
    */
  public:
    /// this is called when we construct the class before everything is initialised.
    Maths_L_Systems(int argc, char **argv) : app(argc, argv) {
    }
    ~Maths_L_Systems()
    {
      fclose(L_Rules);
      delete L_Rules;
      delete var_stack;
      
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene = new visual_scene();
      app_scene->create_default_camera_and_lights();      

      create_base();
      InitDayNightCycle(); 
      app_scene->get_light_instance(0)->get_node()->access_nodeToParent().rotateY(90);
      isday = true;
      gamedaytime = 0.0f;
      nightnumber = 0;

      app_scene->get_camera_instance(0)->set_far_plane(50000);

      scene_node *sunlight_node = new scene_node();
      light *lighttype = new light();
      lighttype->set_near_far(0.1, 15000);
      lighttype->set_color(vec4(1,0,0,1));
      light_instance *sunlight = new light_instance(sunlight_node, lighttype);
      
     sunlight_node->access_nodeToParent().rotateX(45);
      app_scene->add_light_instance(sunlight);

      shader = new param_shader("shaders/default.vs", "shaders/L_SYS.fs");
      mat_tree = new material(vec4(0.48f, 0.337f, 0.062f, 1.0f), shader);


      L_Rules = fopen("L_System_Rules.txt", "r");
      scale = 0.5f;
      initrad = 5.0f;
      initexel = 20.0f;
      curr_itr=1;     
      rad = initrad;
      exel = initexel;
      mat4t dir;
      dir.loadIdentity();      
      dir.translate(0.0f, -10.0f, 0.0f);
      var_stack = new L_stack(rad, exel, dir);   
              
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0,0,50));

      getrule(0);
      iter(curr_itr);
           
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

      int vx = 0, vy = 0;
      
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);
      
      // update matrices. assume 30 fps.
      app_scene->update(1.0f / 30);

      // draw the scene
      app_scene->render((float)vx / vy);
      DayNightCycle();
      user_cont();
    }
  };
}
