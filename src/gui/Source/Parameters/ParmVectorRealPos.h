#import <Foundation/Foundation.h>
#import "ParmRandomVariable.h"



@interface ParmVectorRealPos : ParmRandomVariable <NSCoding> {
    
}

- (id)initWithScaleFactor:(float)sf andTool:(ToolModel*)t;
    
@end
