//
//  HomeViewController.m
//  iOCR
//
//  Created by liqilei on 11/18/13.
//  Copyright (c) 2013 liqilei. All rights reserved.
//

#import "HomeViewController.h"
#import "CameraViewController.h"
#import "ImagePreviewViewController.h"
#import "ImageAdapter.h"
#import "share/Common.h"

@interface HomeViewController ()
@property (strong, nonatomic) CameraViewController* cameraView;
@property (strong, nonatomic) UIImagePickerController* imagePicker;
@property (strong, nonatomic) ImagePreviewViewController* imagePreviewController;

//@property (nonatomic) ResultViewController* resultViewController;
@end

@implementation HomeViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self  && [UIImagePickerController isSourceTypeAvailable:UIImagePickerControllerSourceTypeCamera] ) {
        self.imagePreviewController = [[ImagePreviewViewController alloc] init];
        
        self.cameraView = [[CameraViewController alloc] init];
        
        self.imagePicker = [[UIImagePickerController alloc] init];
        self.imagePicker.sourceType = UIImagePickerControllerSourceTypeCamera;
        self.imagePicker.delegate = self;
        self.imagePicker.cameraOverlayView = self.cameraView.view;
        self.imagePicker.cameraOverlayView.layer.position = CGPointMake(160,240);
        self.imagePicker.showsCameraControls = NO;
        
        CGAffineTransform transform = self.imagePicker.cameraOverlayView.transform;
        transform = CGAffineTransformRotate(transform, (M_PI/2.0));
        self.imagePicker.cameraOverlayView.transform = transform;
        
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(takePhotoDone) name:@"MyTakePhoto" object:nil];
        
        //self.resultViewController = [[ResultViewController alloc] init];

    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString *tessdataPath = [bundlePath stringByAppendingPathComponent:@"tessdata"];
    NSLog(@"%@",tessdataPath);
    
    setenv("TESSDATA_PREFIX", [[bundlePath stringByAppendingString:@"/"] UTF8String], 1);
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskPortrait;
}

- (IBAction)takePhoto:(id)sender {
    [self presentViewController:self.imagePicker animated:NO completion:nil];
    
    //[self presentViewController:self.resultViewController animated:NO completion:nil];
    //self.resultViewController.fpdm.text = @"ASFAFASFASF";

}

- (void) takePhotoDone
{
    [self.imagePicker takePicture];
}

- (void) imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
    UIImage* image = info[UIImagePickerControllerOriginalImage];
    NSLog(@"width = %f",image.size.width);
    NSLog(@"height = %f",image.size.height);
    
    [self dismissViewControllerAnimated:YES completion:^{
        ImageAdapter *adapter = [[ImageAdapter alloc] init];
        self.imagePreviewController.cvImage = [adapter normalToIplImage: image];
        [self presentViewController:self.imagePreviewController animated:NO completion:nil];
    }];
    
}

@end
