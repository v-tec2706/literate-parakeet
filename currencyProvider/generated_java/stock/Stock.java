// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: stock.proto

package stock;

public final class Stock {
  private Stock() {}
  public static void registerAllExtensions(
      com.google.protobuf.ExtensionRegistryLite registry) {
  }

  public static void registerAllExtensions(
      com.google.protobuf.ExtensionRegistry registry) {
    registerAllExtensions(
        (com.google.protobuf.ExtensionRegistryLite) registry);
  }
  static final com.google.protobuf.Descriptors.Descriptor
    internal_static_stock_currencyData_descriptor;
  static final 
    com.google.protobuf.GeneratedMessageV3.FieldAccessorTable
      internal_static_stock_currencyData_fieldAccessorTable;
  static final com.google.protobuf.Descriptors.Descriptor
    internal_static_stock_stockRequest_descriptor;
  static final 
    com.google.protobuf.GeneratedMessageV3.FieldAccessorTable
      internal_static_stock_stockRequest_fieldAccessorTable;
  static final com.google.protobuf.Descriptors.Descriptor
    internal_static_stock_stockResponse_descriptor;
  static final 
    com.google.protobuf.GeneratedMessageV3.FieldAccessorTable
      internal_static_stock_stockResponse_fieldAccessorTable;

  public static com.google.protobuf.Descriptors.FileDescriptor
      getDescriptor() {
    return descriptor;
  }
  private static  com.google.protobuf.Descriptors.FileDescriptor
      descriptor;
  static {
    java.lang.String[] descriptorData = {
      "\n\013stock.proto\022\005stock\"<\n\014currencyData\022\035\n\004" +
      "curr\030\001 \002(\0162\017.stock.currency\022\r\n\005value\030\002 \002" +
      "(\001\"7\n\014stockRequest\022\'\n\ncurrencies\030\001 \003(\0132\023" +
      ".stock.currencyData\"?\n\rstockResponse\022.\n\021" +
      "updatedCurrencies\030\001 \003(\0132\023.stock.currency" +
      "Data*7\n\010currency\022\007\n\003PLN\020\001\022\007\n\003EUR\020\002\022\007\n\003CH" +
      "F\020\003\022\007\n\003USD\020\004\022\007\n\003GBP\020\0052T\n\rStockExchange\022C" +
      "\n\024getUpdatedCurrencies\022\023.stock.stockRequ" +
      "est\032\024.stock.stockResponse\"\000B\t\n\005stockP\001"
    };
    com.google.protobuf.Descriptors.FileDescriptor.InternalDescriptorAssigner assigner =
        new com.google.protobuf.Descriptors.FileDescriptor.    InternalDescriptorAssigner() {
          public com.google.protobuf.ExtensionRegistry assignDescriptors(
              com.google.protobuf.Descriptors.FileDescriptor root) {
            descriptor = root;
            return null;
          }
        };
    com.google.protobuf.Descriptors.FileDescriptor
      .internalBuildGeneratedFileFrom(descriptorData,
        new com.google.protobuf.Descriptors.FileDescriptor[] {
        }, assigner);
    internal_static_stock_currencyData_descriptor =
      getDescriptor().getMessageTypes().get(0);
    internal_static_stock_currencyData_fieldAccessorTable = new
      com.google.protobuf.GeneratedMessageV3.FieldAccessorTable(
        internal_static_stock_currencyData_descriptor,
        new java.lang.String[] { "Curr", "Value", });
    internal_static_stock_stockRequest_descriptor =
      getDescriptor().getMessageTypes().get(1);
    internal_static_stock_stockRequest_fieldAccessorTable = new
      com.google.protobuf.GeneratedMessageV3.FieldAccessorTable(
        internal_static_stock_stockRequest_descriptor,
        new java.lang.String[] { "Currencies", });
    internal_static_stock_stockResponse_descriptor =
      getDescriptor().getMessageTypes().get(2);
    internal_static_stock_stockResponse_fieldAccessorTable = new
      com.google.protobuf.GeneratedMessageV3.FieldAccessorTable(
        internal_static_stock_stockResponse_descriptor,
        new java.lang.String[] { "UpdatedCurrencies", });
  }

  // @@protoc_insertion_point(outer_class_scope)
}