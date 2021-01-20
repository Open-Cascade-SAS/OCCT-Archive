class OccThreejsViewer
{
  myScene = new THREE.Scene();
  myRaycaster = new THREE.Raycaster();
  myObjects = new Map();
  myLastPicked = null;
  myLastPickedName = "";
  myToDynamicHighlight = true;
  myDynHighlightColor = 0x00FFFF;
  mySelectionColor = 0xFFFF00;
  myGround = null;
  myDirLight = new THREE.DirectionalLight (0xffffff, 1);

  myFpsMeter = new Stats();

  constructor (theCanvas, theCubemap, theAntiAlias)
  {
	var aViewer = this;

    var aGlCtx                   = theCanvas.getContext ('webgl2', { alpha: false, depth: true, antialias: theAntiAlias, preserveDrawingBuffer: true } );
    if (aGlCtx == null) { aGlCtx = theCanvas.getContext ('webgl',  { alpha: false, depth: true, antialias: theAntiAlias, preserveDrawingBuffer: true } ); }
    this.myCamera = new THREE.PerspectiveCamera (45, theCanvas.width / theCanvas.height, 0.1, 1000);
    this.myCamera.position.set (1, 1, 1);

    this.myRenderer = new THREE.WebGLRenderer ({antialias: false, canvas: theCanvas, context: aGlCtx});
    this.myRenderer.autoClear = true;
    this.myRenderer.autoClearColor = true;
    this.myRenderer.autoClearDepth = true;
    this.myRenderer.autoClearStencil = true;
    this.myRenderer.setSize (theCanvas.width, theCanvas.height);
    this.myRenderer.outputEncoding = THREE.sRGBEncoding;
    this.myRenderer.shadowMap.enabled = true;
    //this.myRenderer.shadowMap.type = THREE.PCFSoftShadowMap; // default THREE.PCFShadowMap

    //this.myScene.background = new THREE.Color('black');
    this.setBackgroundCubemap (theCubemap, false);

    this.myControls = new THREE.OrbitControls (this.myCamera, theCanvas);
    this.myControls.target.set (0, 0, -0.2);
    this.myControls.update();
    this.myControls.addEventListener ('change', function() { aViewer.updateView(); });

    this.myDirLight.position.set (-0.098, 0.98, -0.20);
    this.myDirLight.position.normalize()
    this.myDirLight.castShadow = true;
    this.myScene.add (new THREE.HemisphereLight (0xffffff, 0x000000, 0.4));
    this.myScene.add (this.myDirLight);
    //const aShadowHelper = new THREE.CameraHelper (this.myDirLight.shadow.camera); this.myScene.add (aShadowHelper)

    const aPlaneGeom = new THREE.PlaneBufferGeometry (15, 15, 32, 32);
    const aPlaneMat  = new THREE.MeshStandardMaterial ({ color: 0xAAAAAA });
    this.myGround = new THREE.Mesh (aPlaneGeom, aPlaneMat);
    this.myGround.receiveShadow = true;
    this.myGround.position.y = -1.0;
    this.myGround.rotation.x = -1.57;
    this.myScene.add (this.myGround);
    this.updateView();

    theCanvas.addEventListener ('click',     this.onClick, false);
	theCanvas.addEventListener ('mousemove', this.onMouseMove, false);

    // FPS meter
    this.myFpsMeter.showPanel (0);
    document.body.appendChild (this.myFpsMeter.dom);
  }

  toDynamicHighlight()             { return this.myToDynamicHighlight; }
  setDynamicHighlight(theToEnable) { this.myToDynamicHighlight = theToEnable; }

  onMouseMove()
  {
    if (event.buttons !== 0) { return; }

    var aViewer = OccViewerModule;
    if (!aViewer.myToDynamicHighlight) { return; }

    var aTime1 = performance.now();
    event.preventDefault();
    var aMouse = new THREE.Vector2();
    aMouse.x =  (event.clientX / aViewer.myRenderer.domElement.width)  * 2 - 1;
    aMouse.y = -(event.clientY / aViewer.myRenderer.domElement.height) * 2 + 1;
    aViewer.myRaycaster.setFromCamera (aMouse, aViewer.myCamera);
    var anRes = aViewer.myRaycaster.intersectObject (aViewer.myScene, true);
    var aNewPicked = anRes.length > 0 ? anRes[0].object : null;
    if (aNewPicked !== aViewer.myLastPicked)
    {
      aViewer.myLastPickedName = "";
      if (aViewer.myLastPicked !== null)
      {
        aViewer.myLastPicked.material.color.set (aViewer.myLastPicked.material.userData.oldColor);
        aViewer.myLastPicked = null;
      }
      if (aNewPicked !== null)
      {
        for (var aParentIter = aNewPicked; aParentIter != null; aParentIter = aParentIter.parent)
        {
          if (aParentIter.name !== "")
          {
            if (aViewer.myLastPickedName !== "") { aViewer.myLastPickedName = "/" + aViewer.myLastPickedName; }
            aViewer.myLastPickedName = aParentIter.name + aViewer.myLastPickedName;
          }
        }
var aTime2 = performance.now();
console.log ( "New picked '" + aViewer.myLastPickedName + "' in " + ((aTime2 - aTime1) * 0.001) + " s") ///

        aNewPicked.material.userData.oldColor = aNewPicked.material.color.getHex();
        aNewPicked.material.color.set (aViewer.myDynHighlightColor);
        aViewer.myLastPicked = aNewPicked;
      }
      aViewer.updateView();
    }
  }

  onClick()
  {
    var aViewer = OccViewerModule;
    event.preventDefault();
    if (aViewer.myLastPicked !== null)
    {
      aViewer.myLastPicked.material.color.set (aViewer.mySelectionColor);
      aViewer.myLastPicked = null
    }
    aViewer.updateView();
  }

  setBackgroundColor (theR, theG, theB, theToUpdate)
  {
    this.myScene.background = new THREE.Color (theR, theG, theB);
    if (theToUpdate) { this.updateView(); }
  }

  setBackgroundCubemap (theFolderPath, theToUpdate)
  {
    this.myScene.background = new THREE.CubeTextureLoader()
      .setPath (theFolderPath)
      .load (['px.jpg','nx.jpg','py.jpg','ny.jpg','pz.jpg','nz.jpg']);

    /// TODO disable sRGB for consistency with OCCT sample workaround
    //this.myScene.background.encoding = THREE.sRGBEncoding;

    if (theToUpdate) { this.updateView(); }
  }

  /**
   * Fit all/selected objects into view.
   * @param {boolean} theAuto [in] fit selected objects (TRUE) or all objects (FALSE)
   */
  fitAllObjects(theAuto, theToUpdate)
  {
    const aFitOffset = 1.2
    const aBox = new THREE.Box3();
    //this.myScene.traverse (function (theChild) { aBox.expandByObject (theChild) });
    for (let [aKey, anObjIter] of this.myObjects)
    {
      aBox.expandByObject (anObjIter);
    }

    const aSize   = aBox.getSize  (new THREE.Vector3());
    const aCenter = aBox.getCenter(new THREE.Vector3());
console.log (" @@ aSize= " + aSize.x + "x" + aSize.y + "x" + aSize.z) ///

    const aMaxSize = Math.max (aSize.x, aSize.y, aSize.z);
    const aFitHeightDist = aMaxSize / (2 * Math.atan (Math.PI * this.myCamera.fov / 360));
    const aFitWidthDist = aFitHeightDist / this.myCamera.aspect;
    const aDist = aFitOffset * Math.max (aFitHeightDist, aFitWidthDist);

    const aDir = this.myControls.target.clone()
      .sub (this.myCamera.position).normalize().multiplyScalar (aDist);
    this.myControls.maxDistance = aDist * 10;
    this.myControls.target.copy (aCenter);

    this.myCamera.near = aDist / 100;
    this.myCamera.far  = aDist * 100;
    this.myCamera.updateProjectionMatrix();

    this.myCamera.position.copy (this.myControls.target).sub (aDir);
    this.myControls.update();
    if (theToUpdate) { this.updateView(); }
  }

  /**
   * Redraw the view.
   */
  updateView()
  {
    this.myFpsMeter.begin();

    this.myRenderer.render (this.myScene, this.myCamera);

    this.myFpsMeter.end();
  }

  /**
   * Remove named object from viewer.
   * @param {string}  theName [in] object name
   * @param {boolean} theToUpdate [in] immediatly request viewer update
   * @return {boolean} FALSE if object was not found
   */
  removeObject (theName, theToUpdate)
  {
    var anOldObj = this.myObjects.get (theName);
    if (anOldObj !== undefined)
    {
      this.myScene.remove (anOldObj);
      this.myObjects.delete (theName)
      if (theToUpdate) { this.updateView(); }
      return true;
    }
    return false;
  }

  /**
   * Clear all named objects from viewer.
   */
  removeAllObjects (theToUpdate)
  {
    for (let [aKey, anObjIter] of this.myObjects)
    {
      this.myScene.remove (anObjIter);
    }
    this.myObjects.clear();
    if (theToUpdate) { this.updateView(); }
  }

  /** Return TRUE if ground is displayed */
  toShowGround() { return this.myGround.parent === this.myScene; }

  /**
   * Show/hide ground.
   * @param theToShow [in] show or hide flag
   */
  setShowGround (theToShow, theToUpdate)
  {
    if (theToShow)
    {
      if (this.myGround.parent !== this.myScene)
      {
        this.myScene.add (this.myGround);
      }
    }
    else
    {
      this.myScene.remove (this.myGround);
    }
    if (theToUpdate) { this.updateView(); }
  }

  /** Returns TRUE if shadows are turned ON */
  toCastShadows() { return this.myDirLight.castShadow; }

  /** Turn shadows on/off */
  setCastShadows (theUseShadows, theToUpdate)
  {
    this.myDirLight.castShadow = theUseShadows;
    if (theToUpdate) { this.updateView(); }
  }

  /**
   * Open object from the given URL.
   * File will be loaded asynchronously.
   * @param {string} theName      [in] object name
   * @param {string} theModelPath [in] model path
   * @param {boolean} theToExpand [in] expand (explore) model or represent it as single object
   */
  openFromUrl (theName, theModelPath, theToExpand)
  {
    this.removeObject (theName, false);
    var aViewer = this;

    var aTime1 = performance.now();
    const aLoader = new THREE.GLTFLoader();
    //aLoader.setPath();
    aLoader.load (theModelPath, function (theGltf) {
      if (theToExpand)
      {
        theGltf.scene.traverse (function (theChild) {
          if (theChild.isMesh)
          {
            theChild.material = theChild.material.clone(); // duplicate materials to dynamically highlight nodes
            theChild.material.envMap = aViewer.myScene.background;
            theChild.castShadow = true;
            theChild.receiveShadow = true
          }
        });

        var aModelRoot = theGltf.scene;
        aModelRoot.name = theName;
        aViewer.myScene.add (aModelRoot);
        aViewer.myObjects.set (theName, aModelRoot);
        aViewer.fitAllObjects();
        return;
      }

      // merge meshes with common material
      var isFirst = true
      const aMatMap = new Map();
      theGltf.scene.traverse (function (theChild) {
        if (theChild.isMesh)
        {
          var aMatObjects = aMatMap.get (theChild.material.name)
          if (aMatObjects === undefined)
          {
            var aMatObjects = [];
            aMatMap.set (theChild.material.name, aMatObjects)
          }
          aMatObjects.push (theChild)
        }
      });

      var aModelRoot = new THREE.Group();
      aModelRoot.name = theName;
      theGltf.scene.updateMatrixWorld();
      for (let [aMatName, aMeshes] of aMatMap)
      {
        const aGeomList = [];
        for (let i = 0; i < aMeshes.length; ++i)
        {
          const aMesh = aMeshes[i];
          if (aMesh.geometry.applyMatrix4 != undefined)
          {
            aMesh.geometry.applyMatrix4 (aMesh.matrixWorld); // pre-apply transformation
          }
          else
          {
            aMesh.geometry.applyMatrix (aMesh.matrixWorld); // pre-apply transformation
          }
          aGeomList.push (aMesh.geometry);
        }

        const aMaterial = aMeshes[0].material;
        aMaterial.envMap = aViewer.myScene.background;
        const aGeom = THREE.BufferGeometryUtils.mergeBufferGeometries (aGeomList);
        const aNode = new THREE.Mesh (aGeom, aMaterial);
        aNode.name = "";
        aNode.castShadow = true;
        aNode.receiveShadow = true
        aModelRoot.add (aNode);
      }

      aViewer.myScene.add (aModelRoot);
      aViewer.myObjects.set (theName, aModelRoot);
      aViewer.fitAllObjects();

      var aTime2 = performance.now();
      console.log ("glTF '" + theModelPath + "' loading time: " + ((aTime2 - aTime1) * 0.001) + " s");
    });
  }
}

function createOccThreejsViewer (theCanvas, theCubemap, theAntiAlias)
{
  let aViewer = new OccThreejsViewer (theCanvas, theCubemap, theAntiAlias);
  return aViewer;
}
