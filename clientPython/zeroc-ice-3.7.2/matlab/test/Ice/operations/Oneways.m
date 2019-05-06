%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef Oneways
    methods(Static)
        function oneways(p)
            p = p.ice_oneway();

            p.ice_ping();

            p.opVoid();

            p.opIdempotent();

            p.opNonmutating();

            try
                p.opByte(hex2dec('ff'), hex2dec('0f'));
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end
        end
    end
end
