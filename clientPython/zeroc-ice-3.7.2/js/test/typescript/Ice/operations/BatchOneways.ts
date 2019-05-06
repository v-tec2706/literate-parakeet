//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";
const test = TestHelper.test;

export async function run(communicator:Ice.Communicator, prx:Test.MyClassPrx, bidir:boolean)
{
    const bs1 = new Uint8Array(10 * 1024);
    for(let i = 0; i < bs1.length; ++i)
    {
        bs1[i] = 0;
    }

    const batch = Test.MyClassPrx.uncheckedCast(prx.ice_batchOneway());
    await batch.ice_flushBatchRequests();

    test(batch.ice_flushBatchRequests().isCompleted()); // Empty flush
    test(batch.ice_flushBatchRequests().isSent()); // Empty flush
    test(batch.ice_flushBatchRequests().sentSynchronously()); // Empty flush

    for(let i = 0; i < 30; ++i)
    {
        await batch.opByteSOneway(bs1);
    }

    let count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += await prx.opByteSOnewayCallCount();
        await Ice.Promise.delay(10);
    }

    if(batch.ice_getConnection() !== null)
    {
        const batch1 = Test.MyClassPrx.uncheckedCast(prx.ice_batchOneway());
        const batch2 = Test.MyClassPrx.uncheckedCast(prx.ice_batchOneway());

        batch1.ice_ping();
        batch2.ice_ping();
        await batch1.ice_flushBatchRequests();
        await batch1.ice_getConnection().then(c => c.close(Ice.ConnectionClose.GracefullyWithWait));
        batch1.ice_ping();
        batch2.ice_ping();

        await batch1.ice_getConnection();
        await batch2.ice_getConnection();

        batch1.ice_ping();
        await batch1.ice_getConnection().then(c => c.close(Ice.ConnectionClose.GracefullyWithWait));

        batch1.ice_ping();
        batch2.ice_ping();
    }

    const identity = new Ice.Identity();
    identity.name = "invalid";
    const batch3 = batch.ice_identity(identity);
    batch3.ice_ping();
    await batch3.ice_flushBatchRequests();

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3.ice_ping();
    batch.ice_ping();
    await batch.ice_flushBatchRequests();
    await batch.ice_ping();
}
