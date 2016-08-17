// config:
var dir = "ulb";
var left = "L";
var right = "R";

/////

var param;
var left_im, right_im;

var l_K, l_Rt, r_K, r_Rt, w, h, sc, left_im_og, right_im_og;
var l_K_inv, l_Rt_inv, r_K_inv, r_Rt_inv;
var z_far, z_near;

var pt_col = [255, 255, 0];
var pt_rad = 2;
var ln_col = [255, 255, 0];

var l_pt = [10.0, 10.0];
var r_pt = [10.0, 10.0];
var r_ln = [0.0, 0.0, 0.0, 0.0];
var z = 0.0;

var zslider;
var zslider_max = 1000;

function essentialMatrix() {
  var poseTransformation = numeric.dot(l_Rt, r_Rt_inv);
  var t = [poseTransformation[0][3], poseTransformation[1][3], poseTransformation[2][3]];
  var R = numeric.getBlock(poseTransformation, [0, 0], [2, 2]);

  var T = [
    [0.0, -t[2], t[1]],
    [t[2], 0.0, -t[0]],
    [-t[1], t[0], 0.0]
  ];
  var essential = numeric.dot(T, R);
  return essential;
}

function fundamentalMatrix() {
  var essential = essentialMatrix();
  var fundamental = numeric.dot(numeric.transpose(l_K_inv), numeric.dot(essential, r_K_inv));
  return fundamental;
}


function calculateEpipolarLine() {
  var F = fundamentalMatrix();
  var x_L = l_pt[0], y_L = l_pt[1];
  function y_R(x_R) {
    var a = -F[2][2] - F[0][2]*x_L - F[2][0]*x_R - F[0][0]*x_L*x_R - F[1][2]*y_L - F[1][0]*x_R*y_L;
    var b = F[2][1] + F[0][1]*x_L + F[1][1]*y_L;
    return a/b;
  }
  
  var x1 = 0, x2 = w;
  var y1 = y_R(x1), y2 = y_R(x2);

  x1 = x1*sc + right_im_og[0];
  x2 = x2*sc + right_im_og[0];
  y1 = y1*sc + right_im_og[1];
  y2 = y2*sc + right_im_og[1];
  
  r_ln = [x1, y1, x2, y2];
}

function projMat(K) {
  var z_diff = z_far - z_near;
  return [
    [K[0][0], 0.0, K[0][2], 0.0],
    [0.0, K[1][1], K[1][2], 0.0],
    [0.0, 0.0, -z_near/z_diff, (z_far * z_near)/z_diff],
    [0.0, 0.0, 1.0, 0.0]
  ];
}

function calculateCorrespondingPoint() {
  var l_proj = projMat(l_K);
  var r_proj = projMat(r_K);
  
  var l_proj_inv = numeric.inv(l_proj);
  var r_proj_inv = numeric.inv(r_proj);

  var H = numeric.dot(r_proj, numeric.dot(r_Rt, numeric.dot(l_Rt_inv, r_proj_inv)));

  var x_L = l_pt[0], y_L = l_pt[1];

  var x_L_h = [x_L, y_L, z, 1.0];
  var x_R_h = numeric.dot(H, x_L_h);
  x_R_h[0] /= x_R_h[3];
  x_R_h[1] /= x_R_h[3];
  r_pt = [x_R_h[0], x_R_h[1]];
}

function preload() {
  loadJSON("datasets/" + dir + "/param.json", function(par) { param = par; });
  left_im = loadImage("datasets/" + dir + "/" + left + ".jpg");
  right_im = loadImage("datasets/" + dir + "/" + right + ".jpg");
}

function convertRt(Rt_) {
  var t_ = [Rt_[0][3], Rt_[1][3], Rt_[2][3]];
  var R_ = numeric.getBlock(Rt_, [0, 0], [2, 2]);

  var R = R_;
  var t = numeric.dot(R, t_);
  var t = [-t[0], -t[1], -t[2]];

  Rt = numeric.identity(4);
  numeric.setBlock(Rt, [0, 0], [2, 2], R);
  for(var i = 0; i < 3; ++i) Rt[i][3] = t[i];
  return Rt;
}

function setup() {
  l_K = param[left+"_K"];
  l_Rt = param[left+"_Rt"];
  r_K = param[right+"_K"];
  r_Rt = param[right+"_Rt"];
  w = param["w"];
  h = param["h"];
  sc = param["scale"];
  left_im_og = [10, 10];
  right_im_og = [w*sc + 20, 10];
  z_near = param["z_near"];
  z_far = param["z_far"];
  
  if(param["mpeg_intrinsic"]) {
    l_Rt = convertRt(l_Rt);
    r_Rt = convertRt(r_Rt);
  }

  l_K_inv = numeric.inv(l_K);
  l_Rt_inv = numeric.inv(l_Rt);
  r_K_inv = numeric.inv(r_K);
  r_Rt_inv = numeric.inv(r_Rt);

  createCanvas(max(left_im_og[0], right_im_og[0]) + w*sc + 10, max(left_im_og[1], right_im_og[1]) + h*sc + 10 + 40);

  zslider = createSlider(0, 1000, 0);
  zslider.position(30, h*sc + 20);
  zslider.input(updateFromSlider);

  background(255);
}


function updateFromSlider() {
  z = zslider.value() / zslider_max;
  calculateCorrespondingPoint();
  draw();
}


function draw() {
  // clear and redraw...
  clear();
  
  // images
  image(left_im, 0, 0, w, h, left_im_og[0], left_im_og[1], w*sc, h*sc);
  image(right_im, 0, 0, w, h, right_im_og[0], right_im_og[1], w*sc, h*sc);
  
  // left pt
  noStroke();
  fill(pt_col);
  var l_x = l_pt[0]*sc + left_im_og[0];
  var l_y = l_pt[1]*sc + left_im_og[1];
  ellipse(l_x, l_y, 2*pt_rad, 2*pt_rad);
  
  // right pt
  noStroke();
  fill(pt_col);
  var r_x = r_pt[0]*sc + right_im_og[0];
  var r_y = r_pt[1]*sc + right_im_og[1];
  ellipse(r_x, r_y, 2*pt_rad, 2*pt_rad);
  
  // epipolar line
  noFill();
  stroke(ln_col);
  line(r_ln[0], r_ln[1], r_ln[2], r_ln[3]);

  // rect
  noFill();
  stroke(0);
  rect(left_im_og[0], left_im_og[1], w*sc, h*sc);
  rect(right_im_og[0], right_im_og[1], w*sc, h*sc);
  
  // slider text
  fill(0);
  noStroke();
  text("d =", 10, h*sc + 35);
}

function updateFromMouse() {
  if(mouseX < right_im_og[0]) {
    l_pt[0] = (mouseX - left_im_og[0]) / sc;
    l_pt[1] = (mouseY - left_im_og[1]) / sc;
  } else {
    r_pt[0] = (mouseX - right_im_og[0]) / sc;
    r_pt[1] = (mouseY - right_im_og[1]) / sc;
  }
  calculateCorrespondingPoint();
  calculateEpipolarLine();
  draw();
}

function mousePressed() {
  if(mouseY < h*sc + 15) updateFromMouse();
  return true;
}

function mouseDragged() {
  if(mouseY < h*sc + 15) updateFromMouse();
  return true;
}