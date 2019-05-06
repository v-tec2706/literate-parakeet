//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    namespace optional
    {
        namespace Test
        {
            [Serializable]
            public class SerializableClass
            {
                public SerializableClass(int v)
                {
                    _v = v;
                }

                public override bool Equals(object obj)
                {
                    if(obj is SerializableClass)
                    {
                        return _v == (obj as SerializableClass)._v;
                    }

                    return false;
                }

                public override int GetHashCode()
                {
                    return base.GetHashCode();
                }

                private int _v;
            }

        }
    }
}
