function r = Unset()
    % Unset - This function returns a singleton instance that we use as a
    %   sentinel value to indicate an unset optional value.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    r = IceInternal.UnsetI.getInstance();
end
