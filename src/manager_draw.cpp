#include "manager_draw.h"

#include <chrono>   // duration
#include <iostream> // cout
#include <thread>   // thread

#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "ray.h"
#include "rtweekend.h"
#include "sphere.h"
#include "vec3.h"

using namespace std::literals::chrono_literals;

color
ray_color(const ray& r,
          const color& background,
          const hittable& world,
          int depth)
{
  hit_record rec;

  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
    return color(0, 0, 0);

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, infinity, rec))
    return background;

  ray scattered;
  color attenuation;
  color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    return emitted;

  scattered.rgb_ = r.rgb_;

  return emitted +
         attenuation * ray_color(scattered, background, world, depth - 1);
}

void
manager_draw::draw(render_settings const rs,
                   std::function<void(double progress)> notify_progress,
                   std::function<bool()> is_cancelled,
                   std::function<void(QImage)> send_pic)
{
  auto th = std::thread(
    [notify_progress, is_cancelled, send_pic, samples_per_pixel = rs.ray_pp_](
      int img_w, int img_h) {
      QImage image(img_w, img_h, QImage::Format::Format_ARGB32_Premultiplied);
      image.fill(QColor(255, 255, 255));

      // Image
      const auto aspect_ratio = static_cast<double>(img_w) / img_h;
      const int max_depth = 50;

      // World
      hittable_list world;
      auto tex_checker = make_shared<lambertian>(
        make_shared<checker_texture>(color(0, 0, 0), color(1, 1, 1)));
      auto tex_metall = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);
      auto tex_trans = make_shared<dielectric>(1.8);
      auto tex_met_r =
        make_shared<lambertian>(make_shared<solid_color>(color(0.8, 0.6, 0.2)));
      auto tex_met_l = make_shared<metal>(color(0.1, 0.2, 0.5), 0.1);

      world.add(make_shared<sphere>(point3{ 0, -101, 0 }, 100, tex_checker));
      world.add(make_shared<sphere>(point3{ 0, 4, 1 }, 1, tex_trans));
      world.add(make_shared<sphere>(point3{ 2, 0, 0 }, 1, tex_met_r));
      world.add(make_shared<sphere>(point3{ -2, 0, 0 }, 1, tex_met_l));

      hittable_list objects;
      objects.add(make_shared<bvh_node>(world));

      // Camera
      int x = 2;
      camera cam(point3(0, 1 + x, x),
                 point3(0, 1, 0),
                 vec3(0, 1, 0),
                 45,
                 aspect_ratio,
                 4.0);

      color background(1, 1, 1);

      for (int j = img_h - 1; j >= 0; --j) {
        for (int i = 0; i < img_w; ++i) {
          color pixel_color(0, 0, 0);
          for (int s = 0; s < samples_per_pixel; ++s) {
            auto u = (i + random_double()) / (img_w - 1);
            auto v = (j + random_double()) / (img_h - 1);
            ray r = cam.get_ray(u, v);
            r.set_RGB(RGB::R);
            pixel_color.e[0] += ray_color(r, background, world, max_depth).e[0];
            r.set_RGB(RGB::G);
            pixel_color.e[1] += ray_color(r, background, world, max_depth).e[1];
            r.set_RGB(RGB::B);
            pixel_color.e[2] += ray_color(r, background, world, max_depth).e[2];
          }

          auto r = pixel_color.x();
          auto g = pixel_color.y();
          auto b = pixel_color.z();

          // Divide the color by the number of samples and gamma-correct for
          // gamma=2.0.
          auto scale = 1.0 / samples_per_pixel;
          r = sqrt(scale * r);
          g = sqrt(scale * g);
          b = sqrt(scale * b);

          image.setPixelColor(i,
                              j,
                              { static_cast<int>(256 * clamp(r, 0.0, 0.999)),
                                static_cast<int>(256 * clamp(g, 0.0, 0.999)),
                                static_cast<int>(256 * clamp(b, 0.0, 0.999)) });

          double progress =
            100.0 - (100.0 * (j * img_w + img_w - i)) / (img_h * img_w);
          notify_progress(progress);
        }
        if (is_cancelled())
          break;
      }

      image = image.mirrored(false, true);
      send_pic(image);
    },
    rs.width_,
    rs.height_);

  th.detach();
}
