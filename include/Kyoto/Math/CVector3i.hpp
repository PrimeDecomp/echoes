#ifndef _CVECTOR3I
#define _CVECTOR3I

class CVector3i {
public:
  CVector3i(int x, int y, int z);
  
  int GetX() const { return mX; }
  int GetY() const { return mY; }
  int GetZ() const { return mZ; }

private:
  int mX;
  int mY;
  int mZ;
  
  static CVector3i sZeroVector;
  static CVector3i sUpVector;
  static CVector3i sDownVector;
  static CVector3i sLeftVector;
  static CVector3i sRightVector;
  static CVector3i sForwardVector;
  static CVector3i sBackVector;
  static CVector3i sOneVector;
};

bool operator==(const CVector3i& lhs, const CVector3i& rhs);

#endif // _CVECTOR3I
