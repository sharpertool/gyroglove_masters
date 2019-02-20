var NAVTREE =
[
  [ "GyroAccGlove", "index.html", [
    [ "Main Page", "index.html", null ],
    [ "Class List", "annotated.html", [
      [ "CmdProcessor", "class_cmd_processor.html", null ],
      [ "Fifo", "class_fifo.html", null ],
      [ "HardwareSerial", "class_hardware_serial.html", null ],
      [ "I2C_Master", "class_i2_c___master.html", null ],
      [ "I2CNotify", "class_i2_c_notify.html", null ],
      [ "IMU", "class_i_m_u.html", null ],
      [ "IMUBase", "class_i_m_u_base.html", null ],
      [ "Port", "class_port.html", null ],
      [ "PortNotify", "class_port_notify.html", null ],
      [ "Print", "class_print.html", null ],
      [ "IMU::regWrite", "struct_i_m_u_1_1reg_write.html", null ],
      [ "ring_buffer", "structring__buffer.html", null ],
      [ "TimerCntr", "class_timer_cntr.html", null ],
      [ "TimerNotify", "class_timer_notify.html", null ]
    ] ],
    [ "Class Index", "classes.html", null ],
    [ "Class Hierarchy", "hierarchy.html", [
      [ "CmdProcessor", "class_cmd_processor.html", null ],
      [ "Fifo", "class_fifo.html", null ],
      [ "I2C_Master", "class_i2_c___master.html", null ],
      [ "I2CNotify", "class_i2_c_notify.html", [
        [ "IMU", "class_i_m_u.html", null ]
      ] ],
      [ "IMUBase", "class_i_m_u_base.html", [
        [ "IMU", "class_i_m_u.html", null ]
      ] ],
      [ "Port", "class_port.html", null ],
      [ "PortNotify", "class_port_notify.html", null ],
      [ "Print", "class_print.html", [
        [ "HardwareSerial", "class_hardware_serial.html", null ]
      ] ],
      [ "IMU::regWrite", "struct_i_m_u_1_1reg_write.html", null ],
      [ "ring_buffer", "structring__buffer.html", null ],
      [ "TimerCntr", "class_timer_cntr.html", null ],
      [ "TimerNotify", "class_timer_notify.html", [
        [ "IMU", "class_i_m_u.html", null ]
      ] ]
    ] ],
    [ "Class Members", "functions.html", null ],
    [ "File List", "files.html", [
      [ "clksystem.cpp", "clksystem_8cpp.html", null ],
      [ "clksystem.h", "clksystem_8h.html", null ],
      [ "CmdProcessor.cpp", "_cmd_processor_8cpp.html", null ],
      [ "CmdProcessor.h", "_cmd_processor_8h.html", null ],
      [ "cpp_hacks.cpp", "cpp__hacks_8cpp.html", null ],
      [ "cpp_hacks.h", "cpp__hacks_8h.html", null ],
      [ "Documentation.html", "_documentation_8html.html", null ],
      [ "fifo.cpp", "fifo_8cpp.html", null ],
      [ "fifo.h", "fifo_8h.html", null ],
      [ "GyroAcc.cpp", "_gyro_acc_8cpp.html", null ],
      [ "HardwareSerial.cpp", "_hardware_serial_8cpp.html", null ],
      [ "HardwareSerial.h", "_hardware_serial_8h.html", null ],
      [ "I2C_Master.h", "_i2_c___master_8h.html", null ],
      [ "IMU.cpp", "_i_m_u_8cpp.html", null ],
      [ "IMU.h", "_i_m_u_8h.html", null ],
      [ "IMUManager.cpp", "_i_m_u_manager_8cpp.html", null ],
      [ "NewDel.cpp", "_new_del_8cpp.html", null ],
      [ "NewDel.h", "_new_del_8h.html", null ],
      [ "Port.cpp", "_port_8cpp.html", null ],
      [ "Port.h", "_port_8h.html", null ],
      [ "Print.cpp", "_print_8cpp.html", null ],
      [ "Print.h", "_print_8h.html", null ],
      [ "TimerCntr.cpp", "_timer_cntr_8cpp.html", null ],
      [ "TimerCntr.h", "_timer_cntr_8h.html", null ]
    ] ],
    [ "File Members", "globals.html", null ]
  ] ]
];

function createIndent(o,domNode,node,level)
{
  if (node.parentNode && node.parentNode.parentNode)
  {
    createIndent(o,domNode,node.parentNode,level+1);
  }
  var imgNode = document.createElement("img");
  if (level==0 && node.childrenData)
  {
    node.plus_img = imgNode;
    node.expandToggle = document.createElement("a");
    node.expandToggle.href = "javascript:void(0)";
    node.expandToggle.onclick = function() 
    {
      if (node.expanded) 
      {
        $(node.getChildrenUL()).slideUp("fast");
        if (node.isLast)
        {
          node.plus_img.src = node.relpath+"ftv2plastnode.png";
        }
        else
        {
          node.plus_img.src = node.relpath+"ftv2pnode.png";
        }
        node.expanded = false;
      } 
      else 
      {
        expandNode(o, node, false);
      }
    }
    node.expandToggle.appendChild(imgNode);
    domNode.appendChild(node.expandToggle);
  }
  else
  {
    domNode.appendChild(imgNode);
  }
  if (level==0)
  {
    if (node.isLast)
    {
      if (node.childrenData)
      {
        imgNode.src = node.relpath+"ftv2plastnode.png";
      }
      else
      {
        imgNode.src = node.relpath+"ftv2lastnode.png";
        domNode.appendChild(imgNode);
      }
    }
    else
    {
      if (node.childrenData)
      {
        imgNode.src = node.relpath+"ftv2pnode.png";
      }
      else
      {
        imgNode.src = node.relpath+"ftv2node.png";
        domNode.appendChild(imgNode);
      }
    }
  }
  else
  {
    if (node.isLast)
    {
      imgNode.src = node.relpath+"ftv2blank.png";
    }
    else
    {
      imgNode.src = node.relpath+"ftv2vertline.png";
    }
  }
  imgNode.border = "0";
}

function newNode(o, po, text, link, childrenData, lastNode)
{
  var node = new Object();
  node.children = Array();
  node.childrenData = childrenData;
  node.depth = po.depth + 1;
  node.relpath = po.relpath;
  node.isLast = lastNode;

  node.li = document.createElement("li");
  po.getChildrenUL().appendChild(node.li);
  node.parentNode = po;

  node.itemDiv = document.createElement("div");
  node.itemDiv.className = "item";

  node.labelSpan = document.createElement("span");
  node.labelSpan.className = "label";

  createIndent(o,node.itemDiv,node,0);
  node.itemDiv.appendChild(node.labelSpan);
  node.li.appendChild(node.itemDiv);

  var a = document.createElement("a");
  node.labelSpan.appendChild(a);
  node.label = document.createTextNode(text);
  a.appendChild(node.label);
  if (link) 
  {
    a.href = node.relpath+link;
  } 
  else 
  {
    if (childrenData != null) 
    {
      a.className = "nolink";
      a.href = "javascript:void(0)";
      a.onclick = node.expandToggle.onclick;
      node.expanded = false;
    }
  }

  node.childrenUL = null;
  node.getChildrenUL = function() 
  {
    if (!node.childrenUL) 
    {
      node.childrenUL = document.createElement("ul");
      node.childrenUL.className = "children_ul";
      node.childrenUL.style.display = "none";
      node.li.appendChild(node.childrenUL);
    }
    return node.childrenUL;
  };

  return node;
}

function showRoot()
{
  var headerHeight = $("#top").height();
  var footerHeight = $("#nav-path").height();
  var windowHeight = $(window).height() - headerHeight - footerHeight;
  navtree.scrollTo('#selected',0,{offset:-windowHeight/2});
}

function expandNode(o, node, imm)
{
  if (node.childrenData && !node.expanded) 
  {
    if (!node.childrenVisited) 
    {
      getNode(o, node);
    }
    if (imm)
    {
      $(node.getChildrenUL()).show();
    } 
    else 
    {
      $(node.getChildrenUL()).slideDown("fast",showRoot);
    }
    if (node.isLast)
    {
      node.plus_img.src = node.relpath+"ftv2mlastnode.png";
    }
    else
    {
      node.plus_img.src = node.relpath+"ftv2mnode.png";
    }
    node.expanded = true;
  }
}

function getNode(o, po)
{
  po.childrenVisited = true;
  var l = po.childrenData.length-1;
  for (var i in po.childrenData) 
  {
    var nodeData = po.childrenData[i];
    po.children[i] = newNode(o, po, nodeData[0], nodeData[1], nodeData[2],
        i==l);
  }
}

function findNavTreePage(url, data)
{
  var nodes = data;
  var result = null;
  for (var i in nodes) 
  {
    var d = nodes[i];
    if (d[1] == url) 
    {
      return new Array(i);
    }
    else if (d[2] != null) // array of children
    {
      result = findNavTreePage(url, d[2]);
      if (result != null) 
      {
        return (new Array(i).concat(result));
      }
    }
  }
  return null;
}

function initNavTree(toroot,relpath)
{
  var o = new Object();
  o.toroot = toroot;
  o.node = new Object();
  o.node.li = document.getElementById("nav-tree-contents");
  o.node.childrenData = NAVTREE;
  o.node.children = new Array();
  o.node.childrenUL = document.createElement("ul");
  o.node.getChildrenUL = function() { return o.node.childrenUL; };
  o.node.li.appendChild(o.node.childrenUL);
  o.node.depth = 0;
  o.node.relpath = relpath;

  getNode(o, o.node);

  o.breadcrumbs = findNavTreePage(toroot, NAVTREE);
  if (o.breadcrumbs == null)
  {
    o.breadcrumbs = findNavTreePage("index.html",NAVTREE);
  }
  if (o.breadcrumbs != null && o.breadcrumbs.length>0)
  {
    var p = o.node;
    for (var i in o.breadcrumbs) 
    {
      var j = o.breadcrumbs[i];
      p = p.children[j];
      expandNode(o,p,true);
    }
    p.itemDiv.className = p.itemDiv.className + " selected";
    p.itemDiv.id = "selected";
    $(window).load(showRoot);
  }
}

