%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef EncapsEncoder11_InstanceData < handle
    methods
        function obj = EncapsEncoder11_InstanceData(previous)
            if ~isempty(previous)
                previous.next = obj;
            end
            obj.previous = previous;
            obj.next = [];
            obj.indirectionTable = {};
            obj.indirectionMap = containers.Map('KeyType', 'int32', 'ValueType', 'int32');
        end
    end
    properties
        % Instance attributes
        sliceType
        firstSlice
        % Slice attributes
        sliceFlags
        writeSlice    % Position of the slice data members
        sliceFlagsPos % Position of the slice flags
        indirectionTable
        indirectionMap
        previous
        next
    end
end
