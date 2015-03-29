TODO
====

* use an "EdgeManager" to handle all edges between nodes and propagate changes
  in the setup to the data layer
  * use the edgemanager to figure out if a node is selected or not. each
    sink/source needs to register itself to the EdgeManager, which itself
    manages if there is a connection between nodes. thus we have an abstract
    layer from which to extract the connectivity information
* move visualization of items to graphicsnodeview, and graphicsnodescene only to
  handle all the items
* change all occurences of QPoint to QPointF: QGraphicsScene works on a real
  based coordinate system
* coherency in enum-, function-, variable-, class-naming
* change BezierEdge such that it decides the internal anchor nodes for the curve
  depending on if the edge-endpoint is linked to a sink or a source


DONE
====
