%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef BI < Test.B
    methods
        function ice_preMarshal(obj)
            obj.preMarshalInvoked = true;
        end

        function ice_postUnmarshal(obj)
            obj.postUnmarshalInvoked = true;
        end
    end
end
