// Source file for the R3 scene graph class 



// Include files 

#include "R3/R3.h"
#include "R3Scene.h"
#include "R3Bobsled.h"



R3Scene::
R3Scene(void)
  : bbox(R3null_box),
    background(0.5,0.5,0.7,1),
    ambient(0,0,0,1)
{
	// Setup default gravity
	gravity = R3Vector(0,0,0);

  // Setup default camera
  camera.eye = R3zero_point;
  camera.towards = R3negz_vector;
  camera.up = R3posy_vector;
  camera.right = R3posx_vector;
  camera.xfov = 0.0;
  camera.yfov = 0.0;
  camera.neardist = 0.01;
  camera.fardist = 100.0;

  // Create root node
  root = new R3Node();
  root->parent = NULL;
  root->transformation = R3identity_matrix;
  root->material = NULL;
  root->shape = NULL;
  root->bbox = R3null_box;
}



static R3Shape *
ReadShape(FILE *fp, int command_number, const char *filename)
{
  // Initialize result
  R3Shape *shape = NULL;

  // Read shape type
  char shape_type[1024];
  if (fscanf(fp, "%s", shape_type) != 1) {
    fprintf(stderr, "Unable to read shape type at command %d in file %s\n", command_number, filename);
    return NULL;
  }      

  // Read shape args
  if (!strcmp(shape_type, "box")) {
    // Read sphere args
    double x1, y1, z1, x2, y2, z2;
    if (fscanf(fp, "%lf%lf%lf%lf%lf%lf", &x1, &y1, &z1, &x2, &y2, &z2) != 6) {
      fprintf(stderr, "Unable to read sphere args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_BOX_SHAPE;
    shape->box = new R3Box(x1, y1, z1, x2, y2, z2);
    shape->sphere = NULL;
    shape->cylinder = NULL;
    shape->cone = NULL;
    shape->mesh = NULL;
    shape->segment = NULL;
    shape->circle = NULL;
  }
  else if (!strcmp(shape_type, "sphere")) {
    // Read sphere args
    double center_x, center_y, center_z, radius;
    if (fscanf(fp, "%lf%lf%lf%lf", &center_x, &center_y, &center_z, &radius) != 4) {
      fprintf(stderr, "Unable to read sphere args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_SPHERE_SHAPE;
    shape->box = NULL;
    shape->sphere = new R3Sphere(R3Point(center_x, center_y, center_z), radius);
    shape->cylinder = NULL;
    shape->cone = NULL;
    shape->mesh = NULL;
    shape->segment = NULL;
    shape->circle = NULL;
  }
  else if (!strcmp(shape_type, "cylinder")) {
    // Read cylinder args
    double center_x, center_y, center_z, radius, height;
    if (fscanf(fp, "%lf%lf%lf%lf%lf", &center_x, &center_y, &center_z, &radius, &height) != 5) {
      fprintf(stderr, "Unable to read cylinder args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_CYLINDER_SHAPE;
    shape->box = NULL;
    shape->sphere = NULL;
    shape->cylinder = new R3Cylinder(R3Point(center_x, center_y, center_z), radius, height);
    shape->cone = NULL;
    shape->mesh = NULL;
    shape->segment = NULL;
    shape->circle = NULL;
  }
  else if (!strcmp(shape_type, "cone")) {
    // Read cylinder args
    double center_x, center_y, center_z, radius, height;
    if (fscanf(fp, "%lf%lf%lf%lf%lf", &center_x, &center_y, &center_z, &radius, &height) != 5) {
      fprintf(stderr, "Unable to read cone args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_CONE_SHAPE;
    shape->box = NULL;
    shape->sphere = NULL;
    shape->cylinder = NULL;
    shape->cone = new R3Cone(R3Point(center_x, center_y, center_z), radius, height);
    shape->mesh = NULL;
    shape->segment = NULL;
    shape->circle = NULL;
  }
  else if (!strcmp(shape_type, "mesh")) {
    // Read mesh args
    char meshname[1024];
    if (fscanf(fp, "%s", meshname) != 1) {
      fprintf(stderr, "Unable to read mesh args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Get mesh filename
    char buffer[2048];
    strcpy(buffer, filename);
    char *bufferp = strrchr(buffer, '/');
    if (bufferp) *(bufferp+1) = '\0';
    else buffer[0] = '\0';
    strcat(buffer, meshname);
    
    // Create mesh
    R3Mesh *mesh = new R3Mesh();
    if (!mesh) {
      fprintf(stderr, "Unable to allocate mesh\n");
      return 0;
    }
    
    // Read mesh file
    if (!mesh->Read(buffer)) {
      fprintf(stderr, "Unable to read mesh: %s\n", buffer);
      return 0;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_MESH_SHAPE;
    shape->box = NULL;
    shape->sphere = NULL;
    shape->cylinder = NULL;
    shape->cone = NULL;
    shape->mesh = mesh;
    shape->segment = NULL;
    shape->circle = NULL;
  }
  else if (!strcmp(shape_type, "line")) {
    // Read sphere args
    double x1, y1, z1, x2, y2, z2;
    if (fscanf(fp, "%lf%lf%lf%lf%lf%lf", &x1, &y1, &z1, &x2, &y2, &z2) != 6) {
      fprintf(stderr, "Unable to read sphere args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_SEGMENT_SHAPE;
    shape->box = NULL;
    shape->sphere = NULL;
    shape->cylinder = NULL;
    shape->cone = NULL;
    shape->mesh = NULL;
    shape->segment = new R3Segment(R3Point(x1, y1, z1), R3Point(x2, y2, z2));
    shape->circle = NULL;
  }
  else if (!strcmp(shape_type, "circle")) {
    // Read circle args
    double center_x, center_y, center_z, normal_x, normal_y, normal_z, radius;
    if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf", &center_x, &center_y, &center_z, &normal_x, &normal_y, &normal_z, &radius) != 7) {
      fprintf(stderr, "Unable to read circle args at command %d in file %s\n", command_number, filename);
      return NULL;
    }

    // Create shape
    shape = new R3Shape();
    shape->type = R3_CIRCLE_SHAPE;
    shape->box = NULL;
    shape->sphere = NULL;
    shape->cylinder = NULL;
    shape->cone = NULL;
    shape->mesh = NULL;
    shape->segment = NULL;
    shape->circle = new R3Circle(R3Point(center_x, center_y, center_z), radius, R3Vector(normal_x, normal_y, normal_z));
  }
  else {
    // Unrecognized shape type
    fprintf(stderr, "Invalid shape type (%s) at command %d in file %s\n", shape_type, command_number, filename);
    return NULL;
  }

  // Return created shape
  return shape;
}

        
        
int R3Scene::
Read(const char *filename, R3Node *node)
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "r"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Create array of materials
  vector<R3Material *> materials;

  // Create stack of transformations
  vector<R3Matrix> transformations;
  R3Matrix current_transformation = R3identity_matrix;

  // Create default material
  R3Material *default_material = new R3Material();
  default_material->ka = R3Rgb(0.2, 0.2, 0.2, 1);
  default_material->kd = R3Rgb(0.5, 0.5, 0.5, 1);
  default_material->ks = R3Rgb(0.5, 0.5, 0.5, 1);
  default_material->kt = R3Rgb(0.0, 0.0, 0.0, 1);
  default_material->emission = R3Rgb(0, 0, 0, 1);
  default_material->shininess = 10;
  default_material->indexofrefraction = 1;
  default_material->texture = NULL;
  default_material->id = 0;

  // Create stack of group information
  const int max_depth = 1024;
  R3Node *group_nodes[max_depth] = { NULL };
  R3Material *group_materials[max_depth] = { NULL };
  group_nodes[0] = (node) ? node : root;
  group_materials[0] = default_material;
  int depth = 0;

  // Read body
  char cmd[128];
  int command_number = 1;
  vector<R3Particle *> particles_for_springs;
  while (fscanf(fp, "%s", cmd) == 1) {
    if (cmd[0] == '#') {
      // Comment -- read everything until end of line
      do { cmd[0] = fgetc(fp); } while ((cmd[0] >= 0) && (cmd[0] != '\n'));
    }
    else if (!strcmp(cmd, "bobsled")) {
        R3Bobsled *bobsled = new R3Bobsled();

        // Read sink parameters 
        double mass;
        R3Point position;
        R3Vector velocity = R3null_vector; 
        int sled_mat_id, skates_mat_id, helmets_mat_id, masks_mat_id, track_first;
        if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%d%d%d%d%d", &mass, &position[0], &position[1], &position[2],
                    &velocity[0], &velocity[1], &velocity[2], 
                    &sled_mat_id, &skates_mat_id, &helmets_mat_id, &masks_mat_id, &track_first) != 12)
        {
            fprintf(stderr, "Unable to read bobsled at command %d in file %s\n", command_number, filename);
            return 0;
        }
        for (int i = 0; i < NUM_SLEDS; i++)
        {
            R3Shape *sled = ReadShape(fp, command_number, filename);
            if (!sled) {
            	fprintf(stderr, "Unable to read sled body mesh at command %d in file %s\n", command_number, filename);
            	return 0;
            }
            bobsled->sleds.push_back(sled);
        }

        R3Shape *skates = ReadShape(fp, command_number, filename);
        if (!skates) {
            fprintf(stderr, "Unable to sled skate mesh at command %d in file %s\n", command_number, filename);
            return 0;
        }
        R3Shape *helmets = ReadShape(fp, command_number, filename);
        if (!helmets) {
            fprintf(stderr, "Unable to read helmet mesh at command %d in file %s\n", command_number, filename);
            return 0;
        }
        R3Shape *masks = ReadShape(fp, command_number, filename);
        if (!masks) {
            fprintf(stderr, "Unable to read helmet mesh at command %d in file %s\n", command_number, filename);
            return 0;
        }
            
        // Get materials
        R3Material *sled_material = group_materials[depth];
        if (sled_mat_id >= 0) {
            if (sled_mat_id < (int) materials.size()) {
                sled_material = materials[sled_mat_id];
            }
            else {
                fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
                return 0;
            }
        }
        R3Material *skates_material = group_materials[depth];
        if (skates_mat_id >= 0) {
            if (skates_mat_id < (int) materials.size()) {
                skates_material = materials[skates_mat_id];
            }
            else {
                fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
                return 0;
            }
        }
        R3Material *helmets_material = group_materials[depth];
        if (helmets_mat_id >= 0) {
            if (helmets_mat_id < (int) materials.size()) {
                helmets_material = materials[helmets_mat_id];
            }
            else {
                fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
                return 0;
            }
        }
        R3Material *masks_material = group_materials[depth];
        if (masks_mat_id >= 0) {
            if (masks_mat_id < (int) materials.size()) {
                masks_material = materials[masks_mat_id];
            }
            else {
                fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
                return 0;
            }
        }
            
        // Create bobsled
		bobsled->hasWon = false;
        bobsled->mass = mass;
        bobsled->position = position;
        bobsled->velocity = velocity;
		bobsled->position.Transform(current_transformation);
		bobsled->velocity.Transform(current_transformation);
        //bobsled->sled = sled;
        bobsled->position.Transform(current_transformation);
        bobsled->velocity.Transform(current_transformation);
        bobsled->skates = skates;
        bobsled->helmets = helmets;
        bobsled->masks = masks;
        bobsled->sled_material = sled_material;
        bobsled->skates_material = skates_material;
        bobsled->helmets_material = helmets_material;
        bobsled->masks_material = masks_material;

        bobsled->isFalling = false;
        bobsled->timeFalling = 0;

        bobsled->track = track_segments[track_first];

            
        bobsled->transformation = current_transformation;
            
        bobsled->big_percent = 0;
        bobsled->little_theta = 0;
            
        // Add bobsled to scene
        bobsleds.push_back(bobsled);
            
        // Update scene bounding box
        bobsled->bbox = R3null_box;
        bobsled->bbox.Union(bobsled->sleds[0]->mesh->bbox);
        bobsled->bbox.Union(skates->mesh->bbox);
        bobsled->bbox.Union(helmets->mesh->bbox);
        bobsled->bbox.Union(masks->mesh->bbox);
            
        // Provide default camera
        bobsled->bbox.Transform(current_transformation);
        double sled_radius = bobsled->bbox.DiagonalRadius();
        R3Point sled_center = bobsled->bbox.Centroid();
            
        R3Camera *sled_camera = new R3Camera();
        sled_camera->towards = R3Vector(0, 0, -1);
        sled_camera->up = R3Vector(0, 1, 0);
        sled_camera->right = R3Vector(1, 0, 0);
        sled_camera->eye = sled_center  - 7 * sled_radius * sled_camera->towards + 4.0 * sled_radius * sled_camera->up;
        sled_camera->towards = (sled_center - sled_camera->eye);
        sled_camera->towards.Normalize();
        sled_camera->up = R3Vector(sled_camera->right);
        sled_camera->up.Cross(sled_camera->towards);
        sled_camera->xfov = 0.25;
        sled_camera->yfov = 0.25;
        sled_camera->neardist = 350;
        sled_camera->fardist = 100000;
        bobsled->camera3 = sled_camera;

        /*R3Camera * */sled_camera = new R3Camera();
        sled_camera->towards = R3Vector(0, 0, -1);
        sled_camera->up = R3Vector(0, 1, 0);
        sled_camera->right = R3Vector(1, 0, 0);
        sled_camera->eye = sled_center + 0.4 * sled_radius * sled_camera->towards + 0.2 * sled_radius * sled_camera->up;
        sled_camera->xfov = 0.25;
        sled_camera->yfov = 0.25;
        sled_camera->neardist = 3;
        sled_camera->fardist = 100000;
        bobsled->camera1 = sled_camera;
        
        
        
    }
    else if (!strcmp(cmd, "ground")) {
        
        printf("reading ground\n");
        // Read sink parameters

        ground = ReadShape(fp, command_number, filename);
        if (!ground) {
            fprintf(stderr, "Unable to read ground mesh at command %d in file %s\n", command_number, filename);
            return 0;
        }
    }
      
	else if (!strcmp(cmd, "obstacle")) {
	  double impact;
      int m;
    int track_num;
      if (fscanf(fp, "%f%d%d", &impact, &m, &track_num) != 3) {
        fprintf(stderr, "Unable to read obstacle at command %d in file %s\n", command_number, filename);
        return 0;
      }
    

      // Read shape
      R3Shape *obstacle_shape = ReadShape(fp, command_number, filename);
      if (!obstacle_shape) {
        fprintf(stderr, "Unable to read obstacle mesh at command %d in file %s\n", command_number, filename);
        return 0;
      }

	  // get track material
	  R3Material *obstacle_material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          obstacle_material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
          return 0;
        }
      }


	  // Create obstacle
	  R3Obstacle *obstacle = new R3Obstacle();
	  obstacle->obstacle_shape = obstacle_shape;
	  obstacle->impact = impact;
	  obstacle->material = obstacle_material;
	  obstacle->transformation = current_transformation;
    obstacles.push_back(obstacle);
    if (obstacle->obstacle_shape->type == R3_MESH_SHAPE)
      obstacle->type = OBSTACLE_ROCK;
    else if (obstacle->obstacle_shape->type == R3_SPHERE_SHAPE)
      obstacle->type = OBSTACLE_SNOWBALL;
    obstacle->track_num = track_num;
    obstacle->velocity = R3Vector(0, 0, 0); // all obstacles start stationary
    
        
	}
      
    else if (!strcmp(cmd, "track")) {
      // Read sink parameters 
      double cof;
      int type, isCovered, m;
      if (fscanf(fp, "%d%lf%d%d", &type, &cof, &isCovered, &m) != 4) {
        fprintf(stderr, "Unable to read track at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Read shape
      R3Shape *trackshape = ReadShape(fp, command_number, filename);
      if (!trackshape) {
        fprintf(stderr, "Unable to read track mesh at command %d in file %s\n", command_number, filename);
        return 0;
      }

	  // get track material
	  R3Material *track_material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          track_material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create track segment
      R3Track *track = new R3Track();
      track->cof = cof;
      track->isCovered = isCovered;
      track->type = (R3TrackType) type;
      track->material = track_material;
      track->transformation = current_transformation;
      track->track_shape = trackshape;
      track->bbox = trackshape->mesh->bbox;
      track->bbox.Transform(track->transformation);
      track->fog = 0;

	  if (type == TRACK_STRAIGHT) {
		  // set beginning and end track points along central axis
		  R3Point straight_start(0, 0, 25);
		  R3Point straight_end(0, 0, -25);
		  straight_start.Transform(current_transformation);
		  straight_end.Transform(current_transformation);
		  track->start = straight_start;
		  track->end = straight_end;

		  // set end plane of track
		  R3Vector straight_endplane_normal = R3Vector(0, 0, -1);
		  straight_endplane_normal.Transform(current_transformation);
		  R3Plane straight_endplane(straight_end, straight_endplane_normal);
		  track->endPlane = straight_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(0, 1, 0);
		  track->endNormal = R3Vector(0, 1, 0);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector straight_side(-20, 0, 0);
		  straight_side.Transform(current_transformation);
		  track->radius = straight_side.Length();
		  track->side = straight_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3zero_point;
		  track->center_pivot = R3Line(R3null_point, R3zero_vector); 

		  // set other fields
		  track->big_radius = 0;
		  track->next = NULL;
	  }
	  else if (type == TRACK_APPROACH_RIGHT) {
		  // set beginning and end track points along central axis
		  R3Point right_approach_start(0, 0, 25);
		  R3Point right_approach_end(0, 0, -25);
		  right_approach_start.Transform(current_transformation);
		  right_approach_end.Transform(current_transformation);
		  track->start = right_approach_start;
		  track->end = right_approach_end;

		  // set end plane of track
		  R3Vector right_approach_endplane_normal = R3Vector(0, 0, -1);
		  right_approach_endplane_normal.Transform(current_transformation);
		  R3Plane right_approach_endplane(right_approach_end, right_approach_endplane_normal);
		  track->endPlane = right_approach_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(0, 1, 0);
		  track->endNormal = R3Vector(1, 0, 0);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector right_approach_side(-20, 0, 0);
		  right_approach_side.Transform(current_transformation);
		  track->radius = right_approach_side.Length();
		  track->side = right_approach_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3zero_point;
		  track->center_pivot = R3Line(R3null_point, R3zero_vector); 

		  // set other fields
		  track->big_radius = 0;
		  track->next = NULL;
	  }
	  else if (type == TRACK_APPROACH_LEFT) {
		  // set beginning and end track points along central axis
		  R3Point left_approach_start(0, 0, 25);
		  R3Point left_approach_end(0, 0, -25);
		  left_approach_start.Transform(current_transformation);
		  left_approach_end.Transform(current_transformation);
		  track->start = left_approach_start;
		  track->end = left_approach_end;

		  // set end plane of track
		  R3Vector left_approach_endplane_normal = R3Vector(0, 0, -1);
		  left_approach_endplane_normal.Transform(current_transformation);
		  R3Plane left_approach_endplane(left_approach_end, left_approach_endplane_normal);
		  track->endPlane = left_approach_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(0, 1, 0);
		  track->endNormal = R3Vector(-1, 0, 0);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector left_approach_side(-20, 0, 0);
		  left_approach_side.Transform(current_transformation);
		  track->radius = left_approach_side.Length();
		  track->side = left_approach_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3zero_point;
		  track->center_pivot = R3Line(R3null_point, R3zero_vector); 

		  // set other fields
		  track->big_radius = 0;
		  track->next = NULL;
	  }
	  else if (type == TRACK_EXIT_RIGHT) {
		  // set beginning and end track points along central axis
		  R3Point right_approach_start(0, 0, 25);
		  R3Point right_approach_end(0, 0, -25);
		  right_approach_start.Transform(current_transformation);
		  right_approach_end.Transform(current_transformation);
		  track->start = right_approach_start;
		  track->end = right_approach_end;

		  // set end plane of track
		  R3Vector right_approach_endplane_normal = R3Vector(0, 0, -1);
		  right_approach_endplane_normal.Transform(current_transformation);
		  R3Plane right_approach_endplane(right_approach_end, right_approach_endplane_normal);
		  track->endPlane = right_approach_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(1, 0, 0);
		  track->endNormal = R3Vector(0, 1, 0);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector right_approach_side(0, -20, 0);
		  right_approach_side.Transform(current_transformation);
		  track->radius = right_approach_side.Length();
		  track->side = right_approach_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3zero_point;
		  track->center_pivot = R3Line(R3null_point, R3zero_vector); 

		  // set other fields
		  track->big_radius = 0;
		  track->next = NULL;
	  }
	  else if (type == TRACK_EXIT_LEFT) {
		  // set beginning and end track points along central axis
		  R3Point left_approach_start(0, 0, 25);
		  R3Point left_approach_end(0, 0, -25);
		  left_approach_start.Transform(current_transformation);
		  left_approach_end.Transform(current_transformation);
		  track->start = left_approach_start;
		  track->end = left_approach_end;

		  // set end plane of track
		  R3Vector left_approach_endplane_normal = R3Vector(0, 0, -1);
		  left_approach_endplane_normal.Transform(current_transformation);
		  R3Plane left_approach_endplane(left_approach_end, left_approach_endplane_normal);
		  track->endPlane = left_approach_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(-1, 0, 0);
		  track->endNormal = R3Vector(0, 1, 0);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector left_approach_side(0, 20, 0);
		  left_approach_side.Transform(current_transformation);
		  track->radius = left_approach_side.Length();
		  track->side = left_approach_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3zero_point;
		  track->center_pivot = R3Line(R3null_point, R3zero_vector); 

		  // set other fields
		  track->big_radius = 0;
		  track->next = NULL;
	  }
	  else if (type == TRACK_TURN_RIGHT) {
		  
		  // set beginning and end track points along central axis
		  R3Point turn_start(20, 0, 0);
		  R3Point turn_end(70, 0, -50);
		  turn_start.Transform(current_transformation);
		  turn_end.Transform(current_transformation);
		  track->start = turn_start;
		  track->end = turn_end;

		  // set end plane of track
		  R3Vector turn_endplane_normal = R3Vector(1, 0, 0);
		  turn_endplane_normal.Transform(current_transformation);
		  R3Plane turn_endplane(turn_end, turn_endplane_normal);
		  track->endPlane = turn_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(1, 0, 0);
		  track->endNormal = R3Vector(0, 0, 1);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector turn_side(0, -20, 0);
		  turn_side.Transform(current_transformation);
		  track->radius = turn_side.Length();
		  track->side = turn_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3Point(70, 0, 0);
		  track->center_point.Transform(current_transformation);
		  R3Vector center_line(0, -1, 0);
		  center_line.Transform(current_transformation);
		  track->center_pivot = R3Line(track->center_point, center_line); 

		  // set other fields
		  track->big_radius = (track->center_point - track->start).Length();
		  track->next = NULL;
	  }
	  else if (type == TRACK_TURN_LEFT) {
		  
		  // set beginning and end track points along central axis
		  R3Point turn_start(-20, 0, 0);
		  R3Point turn_end(-70, 0, -50);
		  turn_start.Transform(current_transformation);
		  turn_end.Transform(current_transformation);
		  track->start = turn_start;
		  track->end = turn_end;

		  // set end plane of track
		  R3Vector turn_endplane_normal = R3Vector(-1, 0, 0);
		  turn_endplane_normal.Transform(current_transformation);
		  R3Plane turn_endplane(turn_end, turn_endplane_normal);
		  track->endPlane = turn_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(-1, 0, 0);
		  track->endNormal = R3Vector(0, 0, 1);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);
          track->startNormal.Normalize();
          track->endNormal.Normalize();

		  // set side vector of track
		  R3Vector turn_side(0, 20, 0);
		  turn_side.Transform(current_transformation);
		  track->radius = turn_side.Length();
		  track->side = turn_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3Point(-70, 0, 0);
		  track->center_point.Transform(current_transformation);
		  R3Vector center_line(0, 1, 0);
		  center_line.Transform(current_transformation);
		  track->center_pivot = R3Line(track->center_point, center_line); 

		  // set other fields
		  track->big_radius = (track->center_point - track->start).Length();
		  track->next = NULL;
	  }
	  else if (type == TRACK_FINISH) {
		  // set beginning and end track points along central axis
		  R3Point straight_start(0, 0, 25);
		  R3Point straight_end(0, 0, -83);
		  straight_start.Transform(current_transformation);
		  straight_end.Transform(current_transformation);
		  track->start = straight_start;
		  track->end = straight_end;

		  // set end plane of track
		  R3Vector straight_endplane_normal = R3Vector(0, 0, -1);
		  straight_endplane_normal.Transform(current_transformation);
		  R3Plane straight_endplane(straight_end, straight_endplane_normal);
		  track->endPlane = straight_endplane;

		  // set initial along vector for track 
		  track->along = R3Vector(0,0,-1);
		  track->along.Transform(current_transformation);
		  track->along.Normalize();

		  // set track normals at beginning and end of segment
		  track->startNormal = R3Vector(0, 1, 0);
		  track->endNormal = R3Vector(0, 1, 0);
		  track->startNormal.Transform(current_transformation);
		  track->endNormal.Transform(current_transformation);

		  // set side vector of track
		  R3Vector straight_side(-20, 0, 0);
		  straight_side.Transform(current_transformation);
		  track->radius = straight_side.Length();
		  track->side = straight_side;
		  track->side.Normalize();

		  // set center line of large curve
		  track->center_point = R3zero_point;
		  track->center_pivot = R3Line(R3null_point, R3zero_vector); 

		  // set other fields
		  track->big_radius = 0;
		  track->next = NULL;

	  }
      // Add track to scene
	  int NSegments = track_segments.size();
      track_segments.push_back(track);
	  if (NSegments != 0) {
		  track_segments[NSegments - 1]->next = track_segments[NSegments];
	  }

      // Update scene bounding box
        bbox.Union(trackshape->mesh->bbox);
    }
    else if (!strcmp(cmd, "particle")) {
      // Read position and velocity
      R3Point position;
      R3Vector velocity;
      double mass, drag, elasticity, lifetime;
      int fixed, m;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%d%lf%lf%lf%d", 
        &position[0], &position[1], &position[2], 
        &velocity[0], &velocity[1], &velocity[2], 
        &mass, &fixed, &drag, &elasticity, &lifetime, &m) != 12) {
        fprintf(stderr, "Unable to read particle at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at particle command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create particle
      R3Particle *particle = new R3Particle();
      particle->position = position;
      particle->velocity = velocity;
      particle->mass = mass;
      particle->fixed = (fixed) ? true : false;
      particle->drag = drag;
      particle->elasticity = elasticity;
      particle->lifetime = lifetime;
      particle->material = material;   

      // Add particle to scene
      particles.push_back(particle);

      // Update scene bounding box
      bbox.Union(position);

      // Add to list of particles available for springs
      particles_for_springs.push_back(particle);
    }
    else if (!strcmp(cmd, "particle_source")) {
      // Read particle source parameters 
      double mass, drag, elasticity, lifetime;
      double rate, velocity, angle_cutoff;
      int fixed, m;
      if (fscanf(fp, "%lf%d%lf%lf%lf%d%lf%lf%lf", &mass, &fixed, &drag, &elasticity, &lifetime, &m, &rate, &velocity, &angle_cutoff) != 9) {
        fprintf(stderr, "Unable to read particle source at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Read shape
      R3Shape *shape = ReadShape(fp, command_number, filename);
      if (!shape) {
        fprintf(stderr, "Unable to read particle source at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at particle source command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create particle source
      R3ParticleSource *source = new R3ParticleSource();
      source->mass = mass;
      source->fixed = (fixed) ? true : false;
      source->drag = drag;
      source->elasticity = elasticity;
      source->lifetime = lifetime;
      source->material = material;   
      source->rate = rate;
      source->velocity = velocity;
      source->angle_cutoff = angle_cutoff;
      source->shape = shape;   

      // Add particle source to scene
      particle_sources.push_back(source);

      // Update scene bounding box
      /*if (shape->type == R3_SEGMENT_SHAPE) bbox.Union(shape->segment->BBox());
      else if (shape->type == R3_BOX_SHAPE) bbox.Union(*(shape->box));
      else if (shape->type == R3_CIRCLE_SHAPE) bbox.Union(shape->circle->BBox());
      else if (shape->type == R3_SPHERE_SHAPE) bbox.Union(shape->sphere->BBox());
      else if (shape->type == R3_CYLINDER_SHAPE) bbox.Union(shape->cylinder->BBox());
      else if (shape->type == R3_CONE_SHAPE) bbox.Union(shape->cone->BBox());
      else if (shape->type == R3_MESH_SHAPE) bbox.Union(shape->mesh->bbox);*/
    }
    else if (!strcmp(cmd, "particle_sink")) {
      // Read sink parameters 
      double intensity, ca, la, qa;
      if (fscanf(fp, "%lf%lf%lf%lf", &intensity, &ca, &la, &qa) != 4) {
        fprintf(stderr, "Unable to read particle sink at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Read shape
      R3Shape *shape = ReadShape(fp, command_number, filename);
      if (!shape) {
        fprintf(stderr, "Unable to read particle source at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create particle sink
      R3ParticleSink *sink = new R3ParticleSink();
      sink->intensity = intensity;
      sink->constant_attenuation = ca;
      sink->linear_attenuation = la;
      sink->quadratic_attenuation = qa;
      sink->shape = shape;

      // Add particle sink to scene
      particle_sinks.push_back(sink);

      // Update scene bounding box
      if (shape->type == R3_SEGMENT_SHAPE) bbox.Union(shape->segment->BBox());
      else if (shape->type == R3_BOX_SHAPE) bbox.Union(*(shape->box));
      else if (shape->type == R3_CIRCLE_SHAPE) bbox.Union(shape->circle->BBox());
      else if (shape->type == R3_SPHERE_SHAPE) bbox.Union(shape->sphere->BBox());
      else if (shape->type == R3_CYLINDER_SHAPE) bbox.Union(shape->cylinder->BBox());
      else if (shape->type == R3_CONE_SHAPE) bbox.Union(shape->cone->BBox());
      else if (shape->type == R3_MESH_SHAPE) bbox.Union(shape->mesh->bbox);
    }
    else if (!strcmp(cmd, "particle_spring")) {
      // Read gravity parameters 
      int id1, id2;
      double rest_length, ks, kd;
      if (fscanf(fp, "%d%d%lf%lf%lf", &id1, &id2, &rest_length, &ks, &kd) != 5) {
        fprintf(stderr, "Unable to read particle spring at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get particles
      R3Particle *particle1 = particles_for_springs[id1];
      R3Particle *particle2 = particles_for_springs[id2];

      // Create particle spring
      R3ParticleSpring *spring = new R3ParticleSpring();
      spring->particles[0] = particle1;
      spring->particles[1] = particle2;
      spring->rest_length = rest_length;
      spring->ks = ks;
      spring->kd = kd;

      // Insert spring into particles
      particle1->springs.push_back(spring);
      particle2->springs.push_back(spring);

      // Insert spring into scene
      particle_springs.push_back(spring);
    }
    else if (!strcmp(cmd, "particle_gravity")) {
      // Read gravity parameters 
      if (fscanf(fp, "%lf%lf%lf", &gravity[0], &gravity[1], &gravity[2]) != 3) {
        fprintf(stderr, "Unable to read particle gravity at command %d in file %s\n", command_number, filename);
        return 0;
      }
    }
    else if (!strcmp(cmd, "tri")) {
      // Read data
      int m;
      R3Point p1, p2, p3;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf", &m, 
        &p1[0], &p1[1], &p1[2], &p2[0], &p2[1], &p2[2], &p3[0], &p3[1], &p3[2]) != 10) {
        fprintf(stderr, "Unable to read triangle at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at tri command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create mesh
      R3Mesh *mesh = new R3Mesh();
      vector<R3MeshVertex *> vertices;
      vertices.push_back(mesh->CreateVertex(p1, R3zero_vector, R2zero_point));
      vertices.push_back(mesh->CreateVertex(p2, R3zero_vector, R2zero_point));
      vertices.push_back(mesh->CreateVertex(p3, R3zero_vector, R2zero_point));
      mesh->CreateFace(vertices);

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_MESH_SHAPE;
      shape->box = NULL;
      shape->sphere = NULL;
      shape->cylinder = NULL;
      shape->cone = NULL;
      shape->mesh = mesh;
      shape->segment = NULL;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = R3null_box;
      node->bbox.Union(p1);
      node->bbox.Union(p2);
      node->bbox.Union(p3);

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "box")) {
      // Read data
      int m;
      R3Point p1, p2;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf", &m, &p1[0], &p1[1], &p1[2], &p2[0], &p2[1], &p2[2]) != 7) {
        fprintf(stderr, "Unable to read box at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at box command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create box
      R3Box *box = new R3Box(p1, p2);

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_BOX_SHAPE;
      shape->box = box;
      shape->sphere = NULL;
      shape->cylinder = NULL;
      shape->cone = NULL;
      shape->mesh = NULL;
      shape->segment = NULL;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = *box;

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "sphere")) {
      // Read data
      int m;
      R3Point c;
      double r;
      if (fscanf(fp, "%d%lf%lf%lf%lf", &m, &c[0], &c[1], &c[2], &r) != 5) {
        fprintf(stderr, "Unable to read sphere at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at sphere command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create sphere
      R3Sphere *sphere = new R3Sphere(c, r);

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_SPHERE_SHAPE;
      shape->box = NULL;
      shape->sphere = sphere;
      shape->cylinder = NULL;
      shape->cone = NULL;
      shape->mesh = NULL;
      shape->segment = NULL;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = sphere->BBox();

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "cylinder")) {
      // Read data
      int m;
      R3Point c;
      double r, h;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf", &m, &c[0], &c[1], &c[2], &r, &h) != 6) {
        fprintf(stderr, "Unable to read cylinder at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at cyl command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create cylinder
      R3Cylinder *cylinder = new R3Cylinder(c, r, h);

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_CYLINDER_SHAPE;
      shape->box = NULL;
      shape->sphere = NULL;
      shape->cylinder = cylinder;
      shape->cone = NULL;
      shape->mesh = NULL;
      shape->segment = NULL;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = cylinder->BBox();

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "mesh")) {
      // Read data
      int m;
	  int u;
      char meshname[256];
      if (fscanf(fp, "%d%d%s", &m, &u, meshname) != 3) {
        fprintf(stderr, "Unable to parse mesh command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at cone command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Get mesh filename
      char buffer[2048];
      strcpy(buffer, filename);
      char *bufferp = strrchr(buffer, '/');
      if (bufferp) *(bufferp+1) = '\0';
      else buffer[0] = '\0';
      strcat(buffer, meshname);

      // Create mesh
      R3Mesh *mesh = new R3Mesh();
      if (!mesh) {
        fprintf(stderr, "Unable to allocate mesh\n");
        return 0;
      }

      // Read mesh file
      if (!mesh->Read(buffer)) {
        fprintf(stderr, "Unable to read mesh: %s\n", buffer);
        return 0;
      }

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_MESH_SHAPE;
      shape->box = NULL;
      shape->sphere = NULL;
      shape->cylinder = NULL;
      shape->cone = NULL;
      shape->mesh = mesh;
      shape->segment = NULL;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = mesh->bbox;

      // Insert node
      if (u)
	      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "cone")) {
      // Read data
      int m;
      R3Point c;
      double r, h;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf", &m, &c[0], &c[1], &c[2], &r, &h) != 6) {
        fprintf(stderr, "Unable to read cone at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at cone command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create cone
      R3Cone *cone = new R3Cone(c, r, h);

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_CONE_SHAPE;
      shape->box = NULL;
      shape->sphere = NULL;
      shape->cylinder = NULL;
      shape->cone = cone;
      shape->mesh = NULL;
      shape->segment = NULL;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = cone->BBox();

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "line")) {
      // Read data
      int m;
      R3Point p1, p2;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf", &m, &p1[0], &p1[1], &p1[2], &p2[0], &p2[1], &p2[2]) != 7) {
        fprintf(stderr, "Unable to read line at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at line command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // Create segment
      R3Segment *segment = new R3Segment(p1, p2);

      // Create shape
      R3Shape *shape = new R3Shape();
      shape->type = R3_SEGMENT_SHAPE;
      shape->box = NULL;
      shape->sphere = NULL;
      shape->cylinder = NULL;
      shape->cone = NULL;
      shape->mesh = NULL;
      shape->segment = segment;

      // Create shape node
      R3Node *node = new R3Node();
      node->transformation = R3identity_matrix;
      node->material = material;
      node->shape = shape;
      node->bbox = segment->BBox();

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];
    }
    else if (!strcmp(cmd, "begin")) {
      // Read data
      int m;
      double matrix[16];
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &m, 
        &matrix[0], &matrix[1], &matrix[2], &matrix[3], 
        &matrix[4], &matrix[5], &matrix[6], &matrix[7], 
        &matrix[8], &matrix[9], &matrix[10], &matrix[11], 
        &matrix[12], &matrix[13], &matrix[14], &matrix[15]) != 17) {
        fprintf(stderr, "Unable to read begin at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material
      R3Material *material = group_materials[depth];
      if (m >= 0) {
        if (m < (int) materials.size()) {
          material = materials[m];
        }
        else {
          fprintf(stderr, "Invalid material id at cone command %d in file %s\n", command_number, filename);
          return 0;
        }
      }

      // push onto stack and update current transformation
      R3Matrix Rmatrix = R3Matrix(matrix);
      transformations.push_back(Rmatrix);
      current_transformation.Transform(Rmatrix);

      // Create new group node
      R3Node *node = new R3Node();
      node->transformation = R3Matrix(matrix);
      node->material = NULL;
      node->shape = NULL;
      node->bbox = R3null_box;

      // Push node onto stack
      depth++;
      group_nodes[depth] = node;
      group_materials[depth] = material;
    }
    else if (!strcmp(cmd, "end")) {
        printf("starting end\n");
      // Pop node from stack
      R3Node *node = group_nodes[depth];
      depth--;

      // Transform bounding box
      node->bbox.Transform(node->transformation);

      // Insert node
      group_nodes[depth]->bbox.Union(node->bbox);
      group_nodes[depth]->children.push_back(node);
      node->parent = group_nodes[depth];

      R3Matrix Rmatrix = transformations.back();
      transformations.pop_back();
      current_transformation.Transform(Rmatrix.Inverse());
        printf("ending end\n");
    }
    else if (!strcmp(cmd, "material")) {
      // Read data
      R3Rgb ka, kd, ks, kt, e;
      double n, ir;
      char texture_name[256];
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%s", 
          &ka[0], &ka[1], &ka[2], &kd[0], &kd[1], &kd[2], &ks[0], &ks[1], &ks[2], &kt[0], &kt[1], &kt[2], 
          &e[0], &e[1], &e[2], &n, &ir, texture_name) != 18) {
        fprintf(stderr, "Unable to read material at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create material
      R3Material *material = new R3Material();
      material->ka = ka;
      material->kd = kd;
      material->ks = ks;
      material->kt = kt;
      material->emission = e;
      material->shininess = n;
      material->indexofrefraction = ir;
      material->texture = NULL;

      // Read texture
      if (strcmp(texture_name, "0")) {
        // Get texture filename
        char buffer[2048];
        strcpy(buffer, filename);
        char *bufferp = strrchr(buffer, '/');
        if (bufferp) *(bufferp+1) = '\0';
        else buffer[0] = '\0';
        strcat(buffer, texture_name);

        // Read texture image
        material->texture = new R2Image();
        if (!material->texture->Read(buffer)) {
          fprintf(stderr, "Unable to read texture from %s at command %d in file %s\n", buffer, command_number, filename);
          return 0;
        }
      }

      // Insert material
      materials.push_back(material);
    }
    else if (!strcmp(cmd, "dir_light")) {
      // Read data
      R3Rgb c;
      R3Vector d;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf", 
        &c[0], &c[1], &c[2], &d[0], &d[1], &d[2]) != 6) {
        fprintf(stderr, "Unable to read directional light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Normalize direction
      d.Normalize();

      // Create light
      R3Light *light = new R3Light();
      light->type = R3_DIRECTIONAL_LIGHT;
      light->color = c;
      light->position = R3Point(0, 0, 0);
      light->direction = d;
      light->radius = 0;
      light->constant_attenuation = 0;
      light->linear_attenuation = 0;
      light->quadratic_attenuation = 0;
      light->angle_attenuation = 0;
      light->angle_cutoff = M_PI;

      // Insert light
      lights.push_back(light);
    }
    else if (!strcmp(cmd, "point_light")) {
      // Read data
      R3Rgb c;
      R3Point p;
      double ca, la, qa;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &c[0], &c[1], &c[2], &p[0], &p[1], &p[2], &ca, &la, &qa) != 9) {
        fprintf(stderr, "Unable to read point light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create light
      R3Light *light = new R3Light();
      light->type = R3_POINT_LIGHT;
      light->color = c;
      light->position = p;
      light->direction = R3Vector(0, 0, 0);
      light->radius = 0;
      light->constant_attenuation = ca;
      light->linear_attenuation = la;
      light->quadratic_attenuation = qa;
      light->angle_attenuation = 0;
      light->angle_cutoff = M_PI;

      // Insert light
      lights.push_back(light);
    }
    else if (!strcmp(cmd, "spot_light")) {
      // Read data
      R3Rgb c;
      R3Point p;
      R3Vector d;
      double ca, la, qa, sc, sd;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
        &c[0], &c[1], &c[2], &p[0], &p[1], &p[2], &d[0], &d[1], &d[2], &ca, &la, &qa, &sc, &sd) != 14) {
        fprintf(stderr, "Unable to read point light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Normalize direction
      d.Normalize();

      // Create light
      R3Light *light = new R3Light();
      light->type = R3_SPOT_LIGHT;
      light->color = c;
      light->position = p;
      light->direction = d;
      light->radius = 0;
      light->constant_attenuation = ca;
      light->linear_attenuation = la;
      light->quadratic_attenuation = qa;
      light->angle_attenuation = sd;
      light->angle_cutoff = sc;

      // Insert light
      lights.push_back(light);
    }
    else if (!strcmp(cmd, "area_light")) {
      // Read data
      R3Rgb c;
      R3Point p;
      R3Vector d;
      double radius, ca, la, qa;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
        &c[0], &c[1], &c[2], &p[0], &p[1], &p[2], &d[0], &d[1], &d[2], &radius, &ca, &la, &qa) != 13) {
        fprintf(stderr, "Unable to read area light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Normalize direction
      d.Normalize();

      // Create light
      R3Light *light = new R3Light();
      light->type = R3_AREA_LIGHT;
      light->color = c;
      light->position = p;
      light->direction = d;
      light->radius = radius;
      light->constant_attenuation = ca;
      light->linear_attenuation = la;
      light->quadratic_attenuation = qa;
      light->angle_attenuation = 0;
      light->angle_cutoff = M_PI;

      // Insert light
      lights.push_back(light);
    }
    else if (!strcmp(cmd, "camera")) {
      // Read data
      double px, py, pz, dx, dy, dz, ux, uy, uz, xfov, neardist, fardist;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &px, &py, &pz, &dx, &dy, &dz, &ux, &uy, &uz, &xfov, &neardist, &fardist) != 12) {
        fprintf(stderr, "Unable to read camera at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Assign camera
      camera.eye = R3Point(px, py, pz);
      camera.towards = R3Vector(dx, dy, dz);
      camera.towards.Normalize();
      camera.up = R3Vector(ux, uy, uz);
      camera.up.Normalize();
      camera.right = camera.towards % camera.up;
      camera.right.Normalize();
      camera.up = camera.right % camera.towards;
      camera.up.Normalize();
      camera.xfov = xfov;
      camera.yfov = xfov;
      camera.neardist = neardist;
      camera.fardist = fardist;
    }
    else if (!strcmp(cmd, "include")) {
      // Read data
      char scenename[256];
      if (fscanf(fp, "%s", scenename) != 1) {
        fprintf(stderr, "Unable to read include command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get scene filename
      char buffer[2048];
      strcpy(buffer, filename);
      char *bufferp = strrchr(buffer, '/');
      if (bufferp) *(bufferp+1) = '\0';
      else buffer[0] = '\0';
      strcat(buffer, scenename);

      // Read scene from included file
      if (!Read(buffer, group_nodes[depth])) {
        fprintf(stderr, "Unable to read included scene: %s\n", buffer);
        return 0;
      }
    }
    else if (!strcmp(cmd, "background")) {
      // Read data
      double r, g, b;
      if (fscanf(fp, "%lf%lf%lf", &r, &g, &b) != 3) {
        fprintf(stderr, "Unable to read background at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Assign background color
      background = R3Rgb(r, g, b, 1);
    }
    else if (!strcmp(cmd, "ambient")) {
      // Read data
      double r, g, b;
      if (fscanf(fp, "%lf%lf%lf", &r, &g, &b) != 3) {
        fprintf(stderr, "Unable to read ambient at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Assign ambient color
      ambient = R3Rgb(r, g, b, 1);
    }
    else {
      fprintf(stderr, "Unrecognized command %d in file %s: %s\n", command_number, filename, cmd);
      return 0;
    }
	
    // Increment command number
    command_number++;
  }

  // Update bounding box
  bbox.Union(root->bbox);

  // Provide default camera
  if (camera.xfov == 0) {
    double scene_radius = bbox.DiagonalRadius();
    R3Point scene_center = bbox.Centroid();
    camera.towards = R3Vector(0, 0, -1);
    camera.up = R3Vector(0, 1, 0);
    camera.right = R3Vector(1, 0, 0);
    camera.eye = scene_center - 3 * scene_radius * camera.towards;
    camera.xfov = 0.25;
    camera.yfov = 0.25;
    camera.neardist = 0.01 * scene_radius;
    camera.fardist = 100 * scene_radius;
  }

  // Provide default lights
  if (lights.size() == 0) {
    // Create first directional light
    R3Light *light = new R3Light();
    R3Vector direction(-3,-4,-5);
    direction.Normalize();
    light->type = R3_DIRECTIONAL_LIGHT;
    light->color = R3Rgb(1,1,1,1);
    light->position = R3Point(0, 0, 0);
    light->direction = direction;
    light->radius = 0;
    light->constant_attenuation = 0;
    light->linear_attenuation = 0;
    light->quadratic_attenuation = 0;
    light->angle_attenuation = 0;
    light->angle_cutoff = M_PI;
    lights.push_back(light);

    // Create second directional light
    light = new R3Light();
    direction = R3Vector(3,2,3);
    direction.Normalize();
    light->type = R3_DIRECTIONAL_LIGHT;
    light->color = R3Rgb(0.5, 0.5, 0.5, 1);
    light->position = R3Point(0, 0, 0);
    light->direction = direction;
    light->radius = 0;
    light->constant_attenuation = 0;
    light->linear_attenuation = 0;
    light->quadratic_attenuation = 0;
    light->angle_attenuation = 0;
    light->angle_cutoff = M_PI;
    lights.push_back(light);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



