//! Function computes point light shadow attenuation (1.0 means no shadow).
float occLightPointShadow (in samplerCube theShadow,
                           in int  theId,
                           in vec3 thePoint,
                           in vec3 theNormal)
{
  vec4 aPosLightSpace = PosLightSpace[occLight_Index(theId)];
  vec3 aLightDir = thePoint - occLight_Position (theId);
  // convert light-to-fragment vector to a depth value.
  vec3 anAbsVec = abs (aLightDir);
  float aLocalZcomp = max (anAbsVec.x, max (anAbsVec.y, anAbsVec.z));
  // set znear and zfar
  float aRange = occShadowMapRangeParams[theId].y;
  float aNear = occShadowMapRangeParams[theId].x;
  float aFar = aRange <= aNear ? POINTLIGHT_ZFAR : aRange;
  float aNormZComp = (aFar + aNear) / (aFar - aNear) - (2.0 * aFar * aNear) / (aFar - aNear) / aLocalZcomp;
  float aDist = (aNormZComp + 1.0) * 0.5;
  // calculate bias and test depth.
  aLightDir = normalize (aLightDir);
  float aBias = min (occShadowMapSizeBias.y * (1.0 - dot (theNormal, aLightDir)), occShadowMapSizeBias.y * 0.1);
  float aClosestDepth = occTextureCube (theShadow, aLightDir).r;
  float aShadow = (aDist - aBias) > aClosestDepth ? 1.0 : 0.0;
  return 1.0 - aShadow;
}
