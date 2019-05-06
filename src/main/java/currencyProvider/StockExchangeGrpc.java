package stock;

import static io.grpc.MethodDescriptor.generateFullMethodName;
import static io.grpc.stub.ClientCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ClientCalls.asyncClientStreamingCall;
import static io.grpc.stub.ClientCalls.asyncServerStreamingCall;
import static io.grpc.stub.ClientCalls.asyncUnaryCall;
import static io.grpc.stub.ClientCalls.blockingServerStreamingCall;
import static io.grpc.stub.ClientCalls.blockingUnaryCall;
import static io.grpc.stub.ClientCalls.futureUnaryCall;
import static io.grpc.stub.ServerCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ServerCalls.asyncClientStreamingCall;
import static io.grpc.stub.ServerCalls.asyncServerStreamingCall;
import static io.grpc.stub.ServerCalls.asyncUnaryCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.20.0)",
    comments = "Source: stock.proto")
public final class StockExchangeGrpc {

  private StockExchangeGrpc() {}

  public static final String SERVICE_NAME = "stock.StockExchange";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<stock.stockRequest,
      stock.stockResponse> getGetUpdatedCurrenciesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "getUpdatedCurrencies",
      requestType = stock.stockRequest.class,
      responseType = stock.stockResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<stock.stockRequest,
      stock.stockResponse> getGetUpdatedCurrenciesMethod() {
    io.grpc.MethodDescriptor<stock.stockRequest, stock.stockResponse> getGetUpdatedCurrenciesMethod;
    if ((getGetUpdatedCurrenciesMethod = StockExchangeGrpc.getGetUpdatedCurrenciesMethod) == null) {
      synchronized (StockExchangeGrpc.class) {
        if ((getGetUpdatedCurrenciesMethod = StockExchangeGrpc.getGetUpdatedCurrenciesMethod) == null) {
          StockExchangeGrpc.getGetUpdatedCurrenciesMethod = getGetUpdatedCurrenciesMethod = 
              io.grpc.MethodDescriptor.<stock.stockRequest, stock.stockResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(
                  "stock.StockExchange", "getUpdatedCurrencies"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  stock.stockRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.ProtoUtils.marshaller(
                  stock.stockResponse.getDefaultInstance()))
                  .setSchemaDescriptor(new StockExchangeMethodDescriptorSupplier("getUpdatedCurrencies"))
                  .build();
          }
        }
     }
     return getGetUpdatedCurrenciesMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static StockExchangeStub newStub(io.grpc.Channel channel) {
    return new StockExchangeStub(channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static StockExchangeBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    return new StockExchangeBlockingStub(channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static StockExchangeFutureStub newFutureStub(
      io.grpc.Channel channel) {
    return new StockExchangeFutureStub(channel);
  }

  /**
   */
  public static abstract class StockExchangeImplBase implements io.grpc.BindableService {

    /**
     */
    public void getUpdatedCurrencies(stock.stockRequest request,
        io.grpc.stub.StreamObserver<stock.stockResponse> responseObserver) {
      asyncUnimplementedUnaryCall(getGetUpdatedCurrenciesMethod(), responseObserver);
    }

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
          .addMethod(
            getGetUpdatedCurrenciesMethod(),
            asyncUnaryCall(
              new MethodHandlers<
                stock.stockRequest,
                stock.stockResponse>(
                  this, METHODID_GET_UPDATED_CURRENCIES)))
          .build();
    }
  }

  /**
   */
  public static final class StockExchangeStub extends io.grpc.stub.AbstractStub<StockExchangeStub> {
    private StockExchangeStub(io.grpc.Channel channel) {
      super(channel);
    }

    private StockExchangeStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StockExchangeStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new StockExchangeStub(channel, callOptions);
    }

    /**
     */
    public void getUpdatedCurrencies(stock.stockRequest request,
        io.grpc.stub.StreamObserver<stock.stockResponse> responseObserver) {
      asyncUnaryCall(
          getChannel().newCall(getGetUpdatedCurrenciesMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   */
  public static final class StockExchangeBlockingStub extends io.grpc.stub.AbstractStub<StockExchangeBlockingStub> {
    private StockExchangeBlockingStub(io.grpc.Channel channel) {
      super(channel);
    }

    private StockExchangeBlockingStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StockExchangeBlockingStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new StockExchangeBlockingStub(channel, callOptions);
    }

    /**
     */
    public stock.stockResponse getUpdatedCurrencies(stock.stockRequest request) {
      return blockingUnaryCall(
          getChannel(), getGetUpdatedCurrenciesMethod(), getCallOptions(), request);
    }
  }

  /**
   */
  public static final class StockExchangeFutureStub extends io.grpc.stub.AbstractStub<StockExchangeFutureStub> {
    private StockExchangeFutureStub(io.grpc.Channel channel) {
      super(channel);
    }

    private StockExchangeFutureStub(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StockExchangeFutureStub build(io.grpc.Channel channel,
        io.grpc.CallOptions callOptions) {
      return new StockExchangeFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<stock.stockResponse> getUpdatedCurrencies(
        stock.stockRequest request) {
      return futureUnaryCall(
          getChannel().newCall(getGetUpdatedCurrenciesMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_UPDATED_CURRENCIES = 0;

  private static final class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final StockExchangeImplBase serviceImpl;
    private final int methodId;

    MethodHandlers(StockExchangeImplBase serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_GET_UPDATED_CURRENCIES:
          serviceImpl.getUpdatedCurrencies((stock.stockRequest) request,
              (io.grpc.stub.StreamObserver<stock.stockResponse>) responseObserver);
          break;
        default:
          throw new AssertionError();
      }
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req> invoke(
        io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        default:
          throw new AssertionError();
      }
    }
  }

  private static abstract class StockExchangeBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoFileDescriptorSupplier, io.grpc.protobuf.ProtoServiceDescriptorSupplier {
    StockExchangeBaseDescriptorSupplier() {}

    @java.lang.Override
    public com.google.protobuf.Descriptors.FileDescriptor getFileDescriptor() {
      return stock.Stock.getDescriptor();
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.ServiceDescriptor getServiceDescriptor() {
      return getFileDescriptor().findServiceByName("StockExchange");
    }
  }

  private static final class StockExchangeFileDescriptorSupplier
      extends StockExchangeBaseDescriptorSupplier {
    StockExchangeFileDescriptorSupplier() {}
  }

  private static final class StockExchangeMethodDescriptorSupplier
      extends StockExchangeBaseDescriptorSupplier
      implements io.grpc.protobuf.ProtoMethodDescriptorSupplier {
    private final String methodName;

    StockExchangeMethodDescriptorSupplier(String methodName) {
      this.methodName = methodName;
    }

    @java.lang.Override
    public com.google.protobuf.Descriptors.MethodDescriptor getMethodDescriptor() {
      return getServiceDescriptor().findMethodByName(methodName);
    }
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (StockExchangeGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .setSchemaDescriptor(new StockExchangeFileDescriptorSupplier())
              .addMethod(getGetUpdatedCurrenciesMethod())
              .build();
        }
      }
    }
    return result;
  }
}
